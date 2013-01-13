#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_bool_t
xsltp_stylesheet_parser_init(xsltp_t *processor, xsltp_stylesheet_parser_t *stylesheet_parser)
{
    stylesheet_parser->processor = processor;

    if ((stylesheet_parser->parse_lock = xsltp_mutex_init()) == NULL) {
        return FALSE;
    }

    if ((stylesheet_parser->parse_cond = xsltp_cond_init()) == NULL) {
        return FALSE;
    }

    if ((stylesheet_parser->stylesheet_parser_cache = xsltp_stylesheet_parser_cache_create()) == NULL) {
        return FALSE;
    }

    return TRUE;
}

xsltp_stylesheet_parser_t *
xsltp_stylesheet_parser_create(xsltp_t *processor)
{
    xsltp_stylesheet_parser_t *stylesheet_parser;

    if ((stylesheet_parser = xsltp_malloc(sizeof(xsltp_stylesheet_parser_t))) == NULL) {
        return NULL;
    }
    memset(stylesheet_parser, 0, sizeof(xsltp_stylesheet_parser_t));

    if (! xsltp_stylesheet_parser_init(processor, stylesheet_parser)) {
        xsltp_stylesheet_parser_destroy(stylesheet_parser);
        return NULL;
    }

    return stylesheet_parser;
}

void
xsltp_stylesheet_parser_destroy(xsltp_stylesheet_parser_t *stylesheet_parser)
{
    if (stylesheet_parser != NULL) {
        xsltp_stylesheet_parser_cache_destroy(stylesheet_parser->stylesheet_parser_cache);
        xsltp_mutex_destroy(stylesheet_parser->parse_lock);
        xsltp_cond_destroy(stylesheet_parser->parse_cond);
        xsltp_free(stylesheet_parser);
    }
}

xsltp_stylesheet_t *
xsltp_stylesheet_parser_create_stylesheet(char *uri)
{
    xsltp_stylesheet_t *xsltp_stylesheet;

    if ((xsltp_stylesheet = xsltp_malloc(sizeof(xsltp_stylesheet_t))) == NULL) {
        return NULL;
    }
    memset(xsltp_stylesheet, 0, sizeof(xsltp_stylesheet_t));

    xsltp_stylesheet->uri   = strdup(uri);
    xsltp_stylesheet->mtime = time(NULL);

    return xsltp_stylesheet;
}

static void
xsltp_stylesheet_parser_destroy_documents(xsltDocumentPtr doc_list) {
    xsltDocumentPtr doc;
    while (doc_list != NULL) {
#ifdef WITH_DEBUG
		printf("xsltp_stylesheet_parser_destroy_documents: document %s\n", doc_list->doc->URL);
#endif
		doc      = doc_list;
		doc_list = doc_list->next;

		xsltFreeDocumentKeys(doc);
		xmlFree(doc);
	}
}

void
xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet_t *xsltp_stylesheet)
{
    if (xsltp_stylesheet->stylesheet != NULL) {
        if (xsltp_stylesheet->stylesheet->doc != NULL) {
#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_free_stylesheet: _private %s\n", xsltp_stylesheet->uri);
#endif
            xsltp_free(xsltp_stylesheet->stylesheet->doc->_private);
        }

#ifdef WITH_DEBUG
        printf("xsltp_stylesheet_parser_free_stylesheet: stylesheet %s\n", xsltp_stylesheet->uri);
#endif
        xsltFreeStylesheet(xsltp_stylesheet->stylesheet);
    }

    xsltp_stylesheet_parser_destroy_documents(xsltp_stylesheet->doc_list);

    xsltp_free(xsltp_stylesheet->uri);
    xsltp_free(xsltp_stylesheet);
}

xsltp_stylesheet_t *
xsltp_stylesheet_parser_parse_file(xsltp_stylesheet_parser_t *stylesheet_parser, char *uri)
{
    xsltp_stylesheet_t *xsltp_stylesheet;

    if (!stylesheet_parser->processor->stylesheet_caching_enable) {
#ifdef WITH_DEBUG
        printf("xsltp_stylesheet_parser_parse_file: parse file %s\n", uri);
#endif
        xsltp_stylesheet = xsltp_stylesheet_parser_create_stylesheet(uri);
        xsltp_stylesheet->stylesheet = xsltParseStylesheetFile((const xmlChar *) uri);
        if (xsltp_stylesheet->stylesheet == NULL) {
#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_parse_file: stylesheet %s is not parsed\n", uri);
#endif
            xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet);

            return NULL;
        }

        return xsltp_stylesheet;
    }

#ifdef WITH_DEBUG
    printf("xsltp_stylesheet_parser_parse_file: cache lookup %s\n", uri);
#endif

    xsltp_stylesheet = xsltp_stylesheet_parser_cache_lookup(stylesheet_parser->stylesheet_parser_cache, uri);

    if (xsltp_stylesheet != NULL && xsltp_stylesheet->stylesheet == NULL) {
        xsltp_mutex_lock(stylesheet_parser->parse_lock);
        if (!xsltp_stylesheet->creating) {
            xsltp_stylesheet->creating = 1;
            xsltp_mutex_unlock(stylesheet_parser->parse_lock);

#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_parse_file: parse file %s\n", uri);
#endif
/*          time_t t = time(NULL) + 10;
            while (time(NULL) < t) {
                sleep(1);
            }*/
            xsltp_stylesheet->stylesheet = xsltParseStylesheetFile((const xmlChar *) uri);
            if (xsltp_stylesheet->stylesheet == NULL) {
#ifdef WITH_DEBUG
                printf("xsltp_stylesheet_parser_parse_file: stylesheet %s is not parsed\n", uri);
#endif
                xsltp_stylesheet->error = 1;
                xsltp_stylesheet->creating = 0;
            }
#ifdef WITH_DEBUG
            else {
                printf("xsltp_stylesheet_parser_parse_file: stylesheet %s is parsed\n", uri);
            }
#endif
            xsltp_cond_broadcast(stylesheet_parser->parse_cond);
        } else {
            /* wait */
            while (xsltp_stylesheet->stylesheet == NULL && xsltp_stylesheet->error == 0) {
#ifdef WITH_DEBUG
                printf("xsltp_stylesheet_parser_parse_file: wait to parse stylesheet %s\n", uri);
#endif
                xsltp_cond_wait(stylesheet_parser->parse_cond, stylesheet_parser->parse_lock);
            }
            xsltp_mutex_unlock(stylesheet_parser->parse_lock);
        }
        if (xsltp_stylesheet->error)
            xsltp_stylesheet = NULL;
    }

    return xsltp_stylesheet;
}

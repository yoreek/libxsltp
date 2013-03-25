#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_bool_t
xsltp_stylesheet_parser_init(xsltp_t *processor, xsltp_stylesheet_parser_t *stylesheet_parser)
{
    stylesheet_parser->processor = processor;

#ifdef HAVE_THREADS
    if ((stylesheet_parser->parse_lock = xsltp_mutex_init()) == NULL) {
        return FALSE;
    }

    if ((stylesheet_parser->parse_cond = xsltp_cond_init()) == NULL) {
        return FALSE;
    }
#endif

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
#ifdef HAVE_THREADS
        xsltp_mutex_destroy(stylesheet_parser->parse_lock);
        xsltp_cond_destroy(stylesheet_parser->parse_cond);
#endif
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

    xsltp_stylesheet->uri     = strdup(uri);
    xsltp_stylesheet->created = time(NULL);

    return xsltp_stylesheet;
}

static void
xsltp_stylesheet_parser_destroy_documents(xsltDocumentPtr doc_list) {
    xsltDocumentPtr doc;
    while (doc_list != NULL) {
		xsltp_log_debug1("document %s", doc_list->doc->URL);

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
        xsltp_stylesheet_parser_destroy_extra_info(xsltp_stylesheet);

/*
        if (xsltp_stylesheet->stylesheet->doc != NULL) {
            xsltp_log_debug1("_private %s", xsltp_stylesheet->uri);

            xsltp_free(xsltp_stylesheet->stylesheet->doc->_private);
        }
*/

        xsltp_log_debug1("stylesheet %s", xsltp_stylesheet->uri);

        xsltFreeStylesheet(xsltp_stylesheet->stylesheet);
    }

    xsltp_stylesheet_parser_destroy_documents(xsltp_stylesheet->doc_list);

    xsltp_free(xsltp_stylesheet->uri);
    xsltp_free(xsltp_stylesheet);
}

xsltp_bool_t
xsltp_stylesheet_parser_create_extra_info(xsltp_stylesheet_t *xsltp_stylesheet)
{
    xsltStylesheetPtr                stylesheet;
    xmlDocPtr                        doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;

    stylesheet = xsltp_stylesheet->stylesheet;
    while (stylesheet != NULL) {
        doc            = stylesheet->doc;

        if ((doc_extra_info = xsltp_malloc(sizeof(xsltp_xml_document_extra_info_t))) == NULL) {
            perror("Memory allocation error");
            return FALSE;
        }

        xsltp_log_debug1("stylesheet: %s", doc->URL);

        doc_extra_info->mtime = xsltp_last_modify((const char *) doc->URL);
        doc->_private         = doc_extra_info;

        if (stylesheet->imports != NULL) {
            stylesheet = stylesheet->imports;
        } else if (stylesheet->next != NULL) {
            stylesheet = stylesheet->next;
        } else {
            while ((stylesheet = stylesheet->parent) != NULL) {
                if (stylesheet->next != NULL) {
                    stylesheet = stylesheet->next;
                    break;
                }
            }
        }
    }

    return TRUE;
}

void
xsltp_stylesheet_parser_destroy_extra_info(xsltp_stylesheet_t *xsltp_stylesheet)
{
    xsltStylesheetPtr                stylesheet;
    xmlDocPtr                        doc;

    stylesheet = xsltp_stylesheet->stylesheet;
    while (stylesheet != NULL) {
        doc = stylesheet->doc;

        xsltp_log_debug1("stylesheet: %s", doc->URL);

        if (doc->_private != NULL) {
            xsltp_free(doc->_private);
        }

        if (stylesheet->imports != NULL) {
            stylesheet = stylesheet->imports;
        } else if (stylesheet->next != NULL) {
            stylesheet = stylesheet->next;
        } else {
            while ((stylesheet = stylesheet->parent) != NULL) {
                if (stylesheet->next != NULL) {
                    stylesheet = stylesheet->next;
                    break;
                }
            }
        }
    }
}

xsltp_stylesheet_t *
xsltp_stylesheet_parser_parse_file(xsltp_stylesheet_parser_t *stylesheet_parser, char *uri)
{
    xsltp_stylesheet_t *xsltp_stylesheet;

    if (!stylesheet_parser->processor->stylesheet_caching_enable) {
        xsltp_log_debug1("parse file %s", uri);

        xsltp_stylesheet = xsltp_stylesheet_parser_create_stylesheet(uri);
        xsltp_stylesheet->stylesheet = xsltParseStylesheetFile((const xmlChar *) uri);
        if (xsltp_stylesheet->stylesheet == NULL) {
            xsltp_log_debug1("stylesheet %s is not parsed", uri);

            xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet);

            return NULL;
        }

        return xsltp_stylesheet;
    }

    xsltp_log_debug1("cache lookup %s", uri);

    xsltp_stylesheet = xsltp_stylesheet_parser_cache_lookup(stylesheet_parser->stylesheet_parser_cache, uri);

    if (xsltp_stylesheet != NULL && xsltp_stylesheet->stylesheet == NULL) {
#ifdef HAVE_THREADS

        xsltp_mutex_lock(stylesheet_parser->parse_lock);
        if (!xsltp_stylesheet->creating) {
            xsltp_stylesheet->creating = 1;
            xsltp_mutex_unlock(stylesheet_parser->parse_lock);

            xsltp_log_debug1("parse file %s", uri);

            xsltp_stylesheet->stylesheet = xsltParseStylesheetFile((const xmlChar *) uri);
            if (xsltp_stylesheet->stylesheet == NULL) {
                xsltp_log_debug1("stylesheet %s is not parsed", uri);

                xsltp_stylesheet->error = 1;
                xsltp_stylesheet->creating = 0;
            }
            else {
                xsltp_log_debug1("stylesheet %s is parsed", uri);

                xsltp_stylesheet_parser_create_extra_info(xsltp_stylesheet);
            }

            xsltp_cond_broadcast(stylesheet_parser->parse_cond);
        } else {
            /* wait */
            while (xsltp_stylesheet->stylesheet == NULL && xsltp_stylesheet->error == 0) {
                xsltp_log_debug1("wait to parse stylesheet %s", uri);

                xsltp_cond_wait(stylesheet_parser->parse_cond, stylesheet_parser->parse_lock);
            }
            xsltp_mutex_unlock(stylesheet_parser->parse_lock);
        }

#else

        xsltp_stylesheet->stylesheet = xsltParseStylesheetFile((const xmlChar *) uri);
        if (xsltp_stylesheet->stylesheet == NULL) {
            xsltp_log_debug1("stylesheet %s is not parsed", uri);

            xsltp_stylesheet->error = 1;
            xsltp_stylesheet->creating = 0;
        }
        else {
            xsltp_log_debug1("stylesheet %s is parsed", uri);

            xsltp_stylesheet_parser_create_extra_info(xsltp_stylesheet);
        }

#endif /* HAVE_THREADS */

        if (xsltp_stylesheet->error)
            xsltp_stylesheet = NULL;
    }

    return xsltp_stylesheet;
}

#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltDocLoaderFunc xsltp_document_parser_loader_func_original = NULL;

static xmlDocPtr
xsltp_document_parser_load_document(const xmlChar * URI, xmlDictPtr dict,
    int options, void *ctxt ATTRIBUTE_UNUSED, xsltLoadType type ATTRIBUTE_UNUSED)
{
    xmlDocPtr doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;
    if ((doc_extra_info = xsltp_malloc(sizeof(xsltp_xml_document_extra_info_t))) == NULL)
        return NULL;

    doc = xsltp_document_parser_loader_func_original(URI, dict, options, ctxt, type);
    if (doc == NULL)
        return NULL;

    doc->_private = doc_extra_info;
    doc_extra_info->mtime = xsltp_last_modify(doc->URL);

    return doc;
}

static xmlDocPtr
xsltp_document_parser_loader_func(const xmlChar * URI, xmlDictPtr dict,
    int options, void *ctxt, xsltLoadType type ATTRIBUTE_UNUSED)
{
    xsltp_document_parser_t *document_parser;
    xsltp_document_t *xsltp_document;
    char *uri = (char *) URI;

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_loader_func: load document: %s\n", uri);
#endif

    /* !!! don't cache stylesheets */
    if (ctxt == NULL || type != XSLT_LOAD_DOCUMENT) {
#ifdef WITH_DEBUG
        printf("xsltp_document_parser_loader_func: default parser for '%s'\n", uri);
#endif
        return xsltp_document_parser_loader_func_original(URI, dict, options, ctxt, type);
    }

    xsltp_t *processor = ((xsltTransformContextPtr) ctxt)->_private;
    /* it is not processor */
    if (processor->id != XSLTP_PROCESSOR_ID) {
#ifdef WITH_DEBUG
        printf("xsltp_document_parser_loader_func: wrong processor id, using default parser for '%s'\n", uri);
#endif
        return xsltp_document_parser_loader_func_original(URI, dict, options, ctxt, type);
    }

    /* caching not enabled */
    if (!processor->document_caching_enable) {
#ifdef WITH_DEBUG
        printf("xsltp_document_parser_loader_func: caching not enabled for '%s'\n", uri);
#endif
        return xsltp_document_parser_loader_func_original(URI, dict, options, ctxt, type);
    }

    document_parser = processor->document_parser;
    xsltp_document = xsltp_document_parser_cache_lookup(document_parser->cache, uri);

    if (xsltp_document != NULL && xsltp_document->doc == NULL) {
#ifdef HAVE_THREADS
        xsltp_mutex_lock(document_parser->parse_lock);

        if (!xsltp_document->creating) {
            xsltp_document->creating = 1;
            xsltp_mutex_unlock(document_parser->parse_lock);

#ifdef WITH_DEBUG
            printf("xsltp_document_parser_loader_func: parse document %s\n", uri);
#endif
            xsltp_document->doc = xsltp_document_parser_load_document(URI, dict, options, ctxt, type);
            if (xsltp_document->doc == NULL) {
#ifdef WITH_DEBUG
                printf("xsltp_document_parser_loader_func: document %s is not parsed\n", uri);
#endif
                xsltp_document->error = 1;
                xsltp_document->creating = 0;
            }
            xsltp_cond_broadcast(document_parser->parse_cond);
        } else {
            /* wait */
            while (xsltp_document->doc == NULL && xsltp_document->error == 0) {
#ifdef WITH_DEBUG
                printf("xsltp_document_parser_loader_func: wait to parse document %s\n", uri);
#endif
                xsltp_cond_wait(document_parser->parse_cond, document_parser->parse_lock);
            }
            xsltp_mutex_unlock(document_parser->parse_lock);
        }
#else
        xsltp_document->doc = xsltp_document_parser_load_document(URI, dict, options, ctxt, type);
        if (xsltp_document->doc == NULL) {
#ifdef WITH_DEBUG
            printf("xsltp_document_parser_loader_func: document %s is not parsed\n", uri);
#endif
            xsltp_document->error = 1;
            xsltp_document->creating = 0;
        }
#endif /* HAVE_THREADS */

        if (xsltp_document->error)
            xsltp_document = NULL;
    }

    if (xsltp_document == NULL)
        return NULL;

    return xsltp_document->doc;
}

xsltp_document_t *
xsltp_document_parser_create_document(char *uri)
{
    xsltp_document_t *xsltp_document;

    if ((xsltp_document = xsltp_malloc(sizeof(xsltp_document_t))) == NULL) {
        return NULL;
    }
    memset(xsltp_document, 0, sizeof(xsltp_document_t));

    xsltp_document->uri = strdup(uri);

    return xsltp_document;
}

static
xsltp_bool_t xsltp_document_parser_loader_init(xsltp_document_parser_t *document_parser)
{
#ifdef HAVE_THREADS
    xsltp_mutex_lock(document_parser->parse_lock);
#endif

    if (xsltp_document_parser_loader_func_original == NULL) {
        xsltp_document_parser_loader_func_original = xsltDocDefaultLoader;
        xsltSetLoaderFunc(xsltp_document_parser_loader_func);
    }

#ifdef HAVE_THREADS
    xsltp_mutex_unlock(document_parser->parse_lock);
#endif

    return TRUE;
}

static xsltp_bool_t
xsltp_document_parser_init(xsltp_t *processor, xsltp_document_parser_t *document_parser)
{
    xsltDocLoaderFunc default_loader;

    document_parser->processor = processor;

#ifdef HAVE_THREADS
    if ((document_parser->parse_lock = xsltp_mutex_init()) == NULL) {
        return FALSE;
    }

    if ((document_parser->parse_cond = xsltp_cond_init()) == NULL) {
        return FALSE;
    }
#endif

    if ((document_parser->cache = xsltp_document_parser_cache_create()) == NULL) {
        return FALSE;
    }

    if (!xsltp_document_parser_loader_init(document_parser)) {
        return FALSE;
    }

    return TRUE;
}

xsltp_document_parser_t *
xsltp_document_parser_create(xsltp_t *processor)
{
    xsltp_document_parser_t *document_parser;

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_create: create a new document parser\n");
#endif

    if ((document_parser = xsltp_malloc(sizeof(xsltp_document_parser_t))) == NULL) {
        return NULL;
    }
    memset(document_parser, 0, sizeof(xsltp_document_parser_t));

    if (! xsltp_document_parser_init(processor, document_parser)) {
        xsltp_document_parser_destroy(document_parser);
        return NULL;
    }

    return document_parser;
}

void
xsltp_document_parser_destroy(xsltp_document_parser_t *document_parser)
{
#ifdef WITH_DEBUG
    printf("xsltp_document_parser_destroy: destroy a document parser\n");
#endif

    if (document_parser != NULL) {
        if (document_parser->cache != NULL) {
            xsltp_document_parser_cache_destroy(document_parser->cache);
        }

        xsltp_free(document_parser);
    }
}

void
xsltp_document_parser_destroy_document(xsltp_document_t *xsltp_document)
{
#ifdef WITH_DEBUG
    printf("xsltp_document_parser_destroy_document: document %s\n", xsltp_document->uri);
#endif

    if (xsltp_document->doc != NULL) {
        xsltp_free(xsltp_document->doc->_private);
        xmlFreeDoc(xsltp_document->doc);
#ifdef WITH_DEBUG
    printf("xsltp_document_parser_destroy_document: doc %p\n", xsltp_document->doc);
#endif
    }
    xsltp_free(xsltp_document->uri);
    xsltp_free(xsltp_document);
}

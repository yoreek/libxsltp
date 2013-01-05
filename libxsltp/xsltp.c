#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static void
xsltp_backup_keys(xsltp_t *processor,
    xsltp_stylesheet_t *xsltp_stylesheet, xsltDocumentPtr *doc_list)
{
    xsltp_keys_t                    *xsltp_keys;
    xmlDocPtr                        doc;
    xsltDocumentPtr                  xslt_document;
    xsltp_xml_document_extra_info_t *doc_extra_info;

#ifdef WITH_DEBUG
    printf("xsltp_backup_keys: stylesheet %s\n", xsltp_stylesheet->uri);
#endif

    for ( ;; ) {
        xslt_document = *doc_list;

        if (xslt_document == NULL || xslt_document->main)
            break;

        doc            = xslt_document->doc;
        doc_extra_info = doc->_private;

#ifdef WITH_DEBUG
        printf("xsltp_backup_keys: document %s\n", doc->URL);
#endif
        xsltp_keys_cache_put(processor->keys_cache, xsltp_stylesheet->uri,
            xsltp_stylesheet->mtime, (char *) doc->URL, doc_extra_info->mtime,
            xslt_document);

        *doc_list = xslt_document->next;
    }
}

static void
xsltp_restore_keys(xsltp_t *processor,
    xsltp_stylesheet_t *xsltp_stylesheet, xsltDocumentPtr *doc_list)
{
    xsltp_list_t      *el, xsltp_keys_list;
    xsltp_keys_t      *xsltp_keys;
    xsltDocumentPtr    xslt_document;

#ifdef WITH_DEBUG
    printf("xsltp_restore_keys: stylesheet %s\n", xsltp_stylesheet->uri);
#endif

    xsltp_keys_cache_get(processor->keys_cache, &xsltp_keys_list,
        xsltp_stylesheet->uri, xsltp_stylesheet->mtime);

    for (
        el  = xsltp_list_first(&xsltp_keys_list);
        el != xsltp_list_end(&xsltp_keys_list);
        el  = xsltp_list_next(el)
    ) {
        xsltp_keys = (xsltp_keys_t *) el;

#ifdef WITH_DEBUG
        printf("xsltp_restore_keys: document %s\n", xsltp_keys->document_uri);
#endif

        if ((xslt_document = xsltp_malloc(sizeof(xsltDocument))) == NULL) {
            return;
        }
        if (xsltp_keys->xslt_document == NULL) {
            memcpy(xslt_document, xsltp_keys->xslt_document_not_computed_keys, sizeof(xsltDocument));
        }
        else {
            memcpy(xslt_document, xsltp_keys->xslt_document, sizeof(xsltDocument));
        }

        xslt_document->next = *doc_list;
        *doc_list           = xslt_document;
    }

    xsltp_keys_cache_free_keys_list(&xsltp_keys_list, XSLT_KEYS_LIST_FREE_NONE);

#ifdef WITH_DEBUG
    printf("xsltp_restore_keys: done\n");
#endif
}

xsltp_result_t *
xsltp_transform(xsltp_t *processor,
    char *stylesheet_uri, xmlDocPtr doc, const char **params)
{
    xsltp_result_t         *result;
    xsltTransformContextPtr ctxt;

#ifdef WITH_DEBUG
    printf("xsltp_transform: start\n");
#endif

    if ((result = xsltp_result_create(processor)) == NULL) {
        return NULL;
    }

    result->xsltp_stylesheet = xsltp_stylesheet_parser_parse_file(processor->stylesheet_parser, stylesheet_uri);

    ctxt = xsltNewTransformContext(result->xsltp_stylesheet->stylesheet, doc);
    ctxt->_private = processor;

    xsltp_restore_keys(processor, result->xsltp_stylesheet, &ctxt->docList);

    result->doc = xsltApplyStylesheetUser(result->xsltp_stylesheet->stylesheet, doc, params, NULL, NULL, ctxt);

    xsltp_backup_keys(processor, result->xsltp_stylesheet, &ctxt->docList);

    xsltFreeTransformContext(ctxt);

    return result;
}

static xsltp_bool_t
xsltp_init(xsltp_t *processor)
{
    xmlInitParser();
    xmlInitThreads();
    /*xsltSetGenericDebugFunc(stderr, NULL);*/
    exsltRegisterAll();
    /*xsltRegisterTestModule();*/
    /*xsltDebugDumpExtensions(NULL);*/

    if ((processor->stylesheet_parser = xsltp_stylesheet_parser_create(processor)) == NULL) {
        return FALSE;
    }

    if ((processor->document_parser = xsltp_document_parser_create(processor)) == NULL) {
        return FALSE;
    }

    if ((processor->keys_cache = xsltp_keys_cache_create(processor)) == NULL) {
        return FALSE;
    }

    return TRUE;
}

xsltp_t *
xsltp_create(xsltp_bool_t stylesheet_cache_enable, xsltp_bool_t document_cache_enable)
{
    xsltp_t *processor;

#ifdef WITH_DEBUG
    printf("xsltp_create: create a new processor\n");
#endif

    if ((processor = xsltp_malloc(sizeof(xsltp_t))) == NULL) {
        return NULL;
    }
    memset(processor, 0, sizeof(xsltp_t));

    processor->stylesheet_cache_enable = stylesheet_cache_enable;
    processor->document_cache_enable   = document_cache_enable;

    if (! xsltp_init(processor)) {
        xsltp_destroy(processor);
        return NULL;
    }

    return processor;
}

void
xsltp_destroy(xsltp_t *processor)
{
#ifdef WITH_DEBUG
    printf("xsltp_destroy: destroy\n");
#endif

    if (processor != NULL) {
        xsltp_stylesheet_parser_destroy(processor->stylesheet_parser);
        xsltp_document_parser_destroy(processor->document_parser);
        xsltp_keys_cache_destroy(processor->keys_cache);
        xsltp_free(processor);

        xsltCleanupGlobals();
/*        xmlCleanupParser();
        xmlMemoryDump();*/
    }

/*
    xmlCleanupThreads();
    xmlCleanupParser();
*/
}

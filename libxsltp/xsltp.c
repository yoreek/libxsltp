#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_bool_t xsltp_initialized = FALSE;

#ifdef HAVE_THREADS
static xsltp_mutex_t xsltp_global_init_lock = XSLTP_THREAD_MUTEX_INITIALIZER;
#endif

static void
xsltp_backup_keys(xsltp_t *processor,
    xsltp_stylesheet_t *xsltp_stylesheet, xsltDocumentPtr *doc_list)
{
    xmlDocPtr                        doc;
    xsltDocumentPtr                  xslt_document;
    xsltp_xml_document_extra_info_t *doc_extra_info;

    xsltp_log_debug1("stylesheet %s", xsltp_stylesheet->uri);

    if (!processor->document_caching_enable) {
        return;
    }

    for ( ;; ) {
        xslt_document = *doc_list;

        if (xslt_document == NULL || xslt_document->main) {
            break;
        }

        *doc_list = xslt_document->next;

        if (processor->keys_caching_enable) {
            if (xslt_document->main) {
                break;
            }

            doc            = xslt_document->doc;
            doc_extra_info = doc->_private;

            xsltp_log_debug1("document %s", doc->URL);

            xsltp_keys_cache_put(processor->keys_cache, xsltp_stylesheet->uri,
                xsltp_stylesheet->created, (char *) doc->URL, doc_extra_info->mtime,
                xslt_document);
        }
        else {
            xsltFreeDocumentKeys(xslt_document);
            xslt_document->doc = NULL;
            free(xslt_document);
        }
    }
}

static void
xsltp_restore_keys(xsltp_t *processor,
    xsltp_stylesheet_t *xsltp_stylesheet, xsltDocumentPtr *doc_list)
{
    xsltp_list_t      *el, xsltp_keys_list;
    xsltp_keys_t      *xsltp_keys;
    xsltDocumentPtr    xslt_document;

    xsltp_log_debug1("stylesheet %s", xsltp_stylesheet->uri);

    if (!processor->document_caching_enable || !processor->keys_caching_enable) {
        return;
    }

    xsltp_keys_cache_get(processor->keys_cache, &xsltp_keys_list,
        xsltp_stylesheet->uri, xsltp_stylesheet->created);

    for (
        el  = xsltp_list_first(&xsltp_keys_list);
        el != xsltp_list_end(&xsltp_keys_list);
        el  = xsltp_list_next(el)
    ) {
        xsltp_keys = (xsltp_keys_t *) el;

        xsltp_log_debug1("document %s", xsltp_keys->document_uri);

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

    xsltp_log_debug0("done");
}

static void
xsltp_reset_profile_info(xsltTransformContextPtr ctxt)
{
    xsltTemplatePtr   template;
    xsltStylesheetPtr style;

    style = ctxt->style;

    while (style != NULL) {
        template = style->templates;

        while (template != NULL) {
            template->nbCalls = 0;
            template->time    = 0;

            template = template->next;
        }

        style = xsltNextImport(style);
    }
}

static xmlDocPtr
xsltp_apply_stylesheet(xsltp_t *processor, xsltp_stylesheet_t *xsltp_stylesheet,
    xmlDocPtr doc, const char **params, FILE *profiler_handle, xmlDocPtr *profiler_info, int pass)
{
    xsltTransformContextPtr ctxt;
    xmlDocPtr               result_doc;

    ctxt = xsltNewTransformContext(xsltp_stylesheet->stylesheet, doc);
    ctxt->_private         = processor;

#ifdef USE_LIBEXSLT_GLOBAL_MAX_DEPTH
    xsltMaxDepth = processor->stylesheet_max_depth;
#else
    ctxt->maxTemplateDepth = processor->stylesheet_max_depth;
#endif

    if (profiler_info != NULL && pass == 1) {
        xsltp_reset_profile_info(ctxt);
    }

    xsltp_restore_keys(processor, xsltp_stylesheet, &ctxt->docList);

    result_doc = xsltApplyStylesheetUser(xsltp_stylesheet->stylesheet,
        doc, params, NULL, profiler_handle, ctxt);

    xsltp_backup_keys(processor, xsltp_stylesheet, &ctxt->docList);

    if (result_doc != NULL && profiler_info != NULL && pass == processor->profiler->repeat) {
        *profiler_info = xsltGetProfileInformation(ctxt);
    }

    xsltFreeTransformContext(ctxt);

    return result_doc;
}

xsltp_result_t *
xsltp_transform(xsltp_t *processor,
    char *stylesheet_uri, xmlDocPtr doc, const char **params,
    xsltp_profiler_result_t *parent_profiler_result)
{
    xsltp_stylesheet_t      *xsltp_stylesheet = NULL;
    xsltp_result_t          *result = NULL;
    xmlDocPtr                result_doc = NULL;
    long                     start = 0, spent = 0;
    int                      i = 1;
    xsltp_profiler_result_t *profiler_result = NULL;
    xmlDocPtr                profiler_info = NULL;

    xsltp_log_debug0("start");

    xsltp_stylesheet = xsltp_stylesheet_parser_parse_file(processor->stylesheet_parser, stylesheet_uri);
    if (xsltp_stylesheet == NULL) {
        goto FAIL;
    }

    result = xsltp_result_create(processor);
    if (result == NULL) {
        goto FAIL;
    }

    if (processor->profiler != NULL) {
        profiler_result = parent_profiler_result;
        if (profiler_result == NULL) {
            profiler_result = xsltp_profiler_result_create(processor);
            if (profiler_result == NULL) {
                goto FAIL;
            }
        }

        start = xsltTimestamp();

        for (i = 1; i <= processor->profiler->repeat; i++) {
            /* cleanup prev result */
            if (result_doc != NULL) {
                xmlFreeDoc(result_doc);
            }

            result_doc = xsltp_apply_stylesheet(processor, xsltp_stylesheet,
                doc, params, processor->profiler->handle, &profiler_info, i
            );
            if (result_doc == NULL) {
                goto FAIL;
            }
        }

        spent = xsltTimestamp() - start;

        if (profiler_info != NULL) {
            xsltp_profiler_result_update(profiler_result, xsltp_stylesheet,
                                         params, spent, doc, profiler_info);

            xmlFreeDoc(profiler_info);
        }
    }
    else {
        result_doc = xsltp_apply_stylesheet(processor, xsltp_stylesheet, doc, params, NULL, NULL, 1);
        if (result_doc == NULL) {
            goto FAIL;
        }
    }

    result->doc              = result_doc;
    result->xsltp_stylesheet = xsltp_stylesheet;

    if (profiler_result != NULL && parent_profiler_result == NULL) {
        result->profiler_result = profiler_result;
        xsltp_profiler_result_apply(processor->profiler, profiler_result, result->doc);
    }

    xsltp_log_debug0("done");

    return result;

FAIL:
    if (result != NULL) {
        xsltp_result_destroy(result);
    }

    if (xsltp_stylesheet != NULL && !processor->stylesheet_caching_enable) {
        xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet);
    }

    xsltp_log_debug0("fail");

    return NULL;
}

xsltp_result_t *
xsltp_transform_multi(xsltp_t *processor, xsltp_transform_ctxt_t *transform_ctxt, xmlDocPtr doc)
{
    int                      i;
    xsltp_result_t          *result = NULL;
    xsltp_profiler_result_t *profiler_result = NULL;

    if (processor->profiler != NULL) {
        profiler_result = xsltp_profiler_result_create(processor);
        if (profiler_result == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < XSLTP_MAX_TRANSFORMATIONS; i++) {
        if (transform_ctxt[i].stylesheet == NULL) {
            break;
        }

        if (result != NULL) {
            doc = result->doc;
            result->doc = NULL;
            xsltp_result_destroy(result);
        }

        result = xsltp_transform(processor, transform_ctxt[i].stylesheet,
            doc, (const char **) transform_ctxt[i].params, profiler_result);

        if (i > 0) {
            xmlFreeDoc(doc);
        }

        if (result == NULL) {
            break;
        }
    }

    if (profiler_result != NULL && result != NULL) {
        result->profiler_result = profiler_result;
        xsltp_profiler_result_apply(processor->profiler, profiler_result, result->doc);
    }

    return result;
}

static xsltp_bool_t
xsltp_init(xsltp_t *processor)
{
    if ((processor->stylesheet_parser = xsltp_stylesheet_parser_create(processor)) == NULL) {
        return FALSE;
    }

    if ((processor->document_parser = xsltp_document_parser_create(processor)) == NULL) {
        return FALSE;
    }

    if ((processor->keys_cache = xsltp_keys_cache_create(processor)) == NULL) {
        return FALSE;
    }

    /* defaults */
    processor->stylesheet_max_depth      = 250;
    processor->stylesheet_caching_enable = TRUE;
    processor->document_caching_enable   = TRUE;
    processor->keys_caching_enable       = TRUE;
    processor->profiler_enable           = FALSE;
    processor->profiler_stylesheet       = NULL;
    processor->profiler_repeat           = 1;

    return TRUE;
}

xsltp_t *
xsltp_create(void)
{
    xsltp_t *processor;

    xsltp_log_debug0("create a new processor");

    if ((processor = xsltp_malloc(sizeof(xsltp_t))) == NULL) {
        return NULL;
    }
    memset(processor, 0, sizeof(xsltp_t));

    processor->id = XSLTP_PROCESSOR_ID;

    if (! xsltp_init(processor)) {
        xsltp_destroy(processor);
        return NULL;
    }

    return processor;
}

void
xsltp_destroy(xsltp_t *processor)
{
    xsltp_log_debug0("destroy");

    if (processor != NULL) {
        xsltp_stylesheet_parser_destroy(processor->stylesheet_parser);
        xsltp_document_parser_destroy(processor->document_parser);
        xsltp_keys_cache_destroy(processor->keys_cache);
        xsltp_profiler_destroy(processor->profiler);
        xsltp_free(processor);
    }
}

void
xsltp_global_init(void)
{
    xsltp_log_debug0("init");

#ifdef HAVE_THREADS
    xsltp_mutex_lock(&xsltp_global_init_lock);
#endif

    if (xsltp_initialized) {
#ifdef HAVE_THREADS
        xsltp_mutex_unlock(&xsltp_global_init_lock);
#endif
        return;
    }

    xmlInitParser();
    xmlInitThreads();
    xsltInitGlobals();
#ifdef HAVE_LIBEXSLT
    exsltRegisterAll();
#endif

    xsltp_extension_init();

    /*xsltSetGenericDebugFunc(stderr, NULL);*/
    /*xsltRegisterTestModule();*/
    /*xsltDebugDumpExtensions(NULL);*/

    xsltp_initialized = TRUE;

#ifdef HAVE_THREADS
    xsltp_mutex_unlock(&xsltp_global_init_lock);
#endif
}

void
xsltp_global_cleanup(void)
{
    xsltp_log_debug0("cleanup");

#ifdef HAVE_THREADS
    xsltp_mutex_lock(&xsltp_global_init_lock);
#endif

    if (!xsltp_initialized) {
#ifdef HAVE_THREADS
        xsltp_mutex_unlock(&xsltp_global_init_lock);
#endif
        return;
    }

    xsltCleanupGlobals();
    xmlCleanupThreads();
    xmlCleanupParser();

    xsltp_initialized = FALSE;

#ifdef HAVE_THREADS
    xsltp_mutex_unlock(&xsltp_global_init_lock);
#endif
}

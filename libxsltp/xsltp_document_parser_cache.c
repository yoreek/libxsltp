#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

xsltp_bool_t
xsltp_document_parser_check_if_modify(xsltp_document_t *xsltp_document)
{
    xmlDocPtr                        doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;
    time_t                           cur_mtime;

    doc            = xsltp_document->doc;
    doc_extra_info = doc->_private;

    cur_mtime = xsltp_last_modify((const char *) doc->URL);
    if (doc_extra_info->mtime != cur_mtime) {
        xsltp_log_debug3("document is updated: %s mtime_old: %d mtime: %d", doc->URL, (int) doc_extra_info->mtime, (int) cur_mtime);

        return 1;
    }

    return 0;
}

static xsltp_bool_t
xsltp_document_parser_cache_init(xsltp_document_parser_cache_t *document_parser_cache)
{
#ifdef HAVE_THREADS
    if ((document_parser_cache->cache_lock = xsltp_rwlock_init()) == NULL) {
        return FALSE;
    }

    if ((document_parser_cache->create_lock = xsltp_mutex_init()) == NULL) {
        return FALSE;
    }
#endif

    xsltp_list_init(&document_parser_cache->list);

    return TRUE;
}

xsltp_document_parser_cache_t *
xsltp_document_parser_cache_create(void)
{
    xsltp_document_parser_cache_t *document_parser_cache;

    xsltp_log_debug0("create cache");

    if ((document_parser_cache = xsltp_malloc(sizeof(xsltp_document_parser_cache_t))) == NULL) {
        return NULL;
    }
    memset(document_parser_cache, 0, sizeof(xsltp_document_parser_cache_t));

    if (! xsltp_document_parser_cache_init(document_parser_cache)) {
        xsltp_document_parser_cache_destroy(document_parser_cache);
        return NULL;
    }

    return document_parser_cache;
}

static void
xsltp_document_parser_cache_free(xsltp_list_t *list)
{
    xsltp_list_t *el, *next_el;
    el = xsltp_list_first(list);
    while (el != xsltp_list_end(list)) {
        next_el = xsltp_list_next(el);
        xsltp_document_parser_destroy_document((xsltp_document_t *) el);
        el = next_el;
    }
}

void
xsltp_document_parser_cache_destroy(xsltp_document_parser_cache_t *document_parser_cache)
{
    xsltp_log_debug0("destroy cache");

    if (document_parser_cache != NULL) {
#ifdef HAVE_THREADS
        xsltp_rwlock_wrlock(document_parser_cache->cache_lock);
#endif

        xsltp_document_parser_cache_free(&document_parser_cache->list);

#ifdef HAVE_THREADS
        xsltp_rwlock_unlock(document_parser_cache->cache_lock);
        xsltp_mutex_destroy(document_parser_cache->create_lock);
#endif

        xsltp_free(document_parser_cache);
    }
}

xsltp_document_t *
xsltp_document_parser_cache_lookup(xsltp_document_parser_cache_t *document_parser_cache,
    char *uri)
{
    xsltp_document_t *xsltp_document = NULL, *xsltp_document_old = NULL;
    xsltp_list_t          *el;
    int               is_found = 0;

    xsltp_log_debug1("lookup document %s in cache", uri);

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(document_parser_cache->cache_lock);
#endif

    for (
        el = xsltp_list_first(&document_parser_cache->list);
        el != xsltp_list_end(&document_parser_cache->list);
        el = xsltp_list_next(el)
    ) {
        xsltp_document = (xsltp_document_t *) el;

        if (strcmp(xsltp_document->uri, uri) == 0) {
            if (xsltp_document->expired == 0 && xsltp_document->error == 0) {
                xsltp_log_debug1("document %s is found in cache", uri);

                is_found = 1;
                break;
            }

            if (xsltp_document->expired && xsltp_document->doc != NULL) {
                xsltp_document_old = xsltp_document;
            }
        }
    }

    if (!is_found) {
        xsltp_document = xsltp_document_parser_create_document(uri);
        xsltp_list_insert_tail(&document_parser_cache->list, (xsltp_list_t *) xsltp_document);
    } else if (xsltp_document->doc == NULL && xsltp_document_old != NULL) {
        xsltp_log_debug1("document(expired) %s is found in cache", uri);

        xsltp_document = xsltp_document_old;
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(document_parser_cache->cache_lock);
#endif

    return xsltp_document;
}

void
xsltp_document_parser_cache_clean(xsltp_document_parser_cache_t *document_parser_cache, xsltp_keys_cache_t *keys_cache)
{
    xsltp_document_t           *xsltp_document;
    xsltp_xml_document_extra_info_t *doc_extra_info;
    xmlDocPtr                   doc;
    time_t                      now;
    xsltp_list_t               *el;

    xsltp_log_debug0("start");

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(document_parser_cache->cache_lock);
#endif

    now = time(NULL);
    el = xsltp_list_first(&document_parser_cache->list);
    while (el != xsltp_list_end(&document_parser_cache->list)) {
        xsltp_document = (xsltp_document_t *) el;
        doc               = xsltp_document->doc;
        doc_extra_info    = doc->_private;

        if (
            xsltp_document->expired == 0 &&
            xsltp_document_parser_check_if_modify(xsltp_document)
        ) {
            xsltp_log_debug1("document %s is expired", xsltp_document->uri);

            xsltp_document->expired = now;
            xsltp_keys_cache_expire(keys_cache, NULL, 0, xsltp_document->uri, doc_extra_info->mtime);
        }

        if (
            xsltp_document->expired != 0 &&
            (now - xsltp_document->expired) > 0
        ) {
            /* clean */
            xsltp_list_remove(el);
            el = xsltp_list_next(el);

            xsltp_log_debug1("free document %s", xsltp_document->uri);

            xsltp_document_parser_destroy_document(xsltp_document);
        } else {
            el = xsltp_list_next(el);
        }
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(document_parser_cache->cache_lock);
#endif

    xsltp_log_debug0("end");
}

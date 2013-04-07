#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_bool_t
xsltp_stylesheet_parser_cache_init(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache)
{
#ifdef HAVE_THREADS
    if ((stylesheet_parser_cache->cache_lock = xsltp_rwlock_init()) == NULL) {
        return FALSE;
    }

    if ((stylesheet_parser_cache->create_lock = xsltp_mutex_init()) == NULL) {
        return FALSE;
    }
#endif

    xsltp_list_init(&stylesheet_parser_cache->list);

    return TRUE;
}

xsltp_stylesheet_parser_cache_t *
xsltp_stylesheet_parser_cache_create(void)
{
    xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache;

    if ((stylesheet_parser_cache = xsltp_malloc(sizeof(xsltp_stylesheet_parser_cache_t))) == NULL) {
        return NULL;
    }
    memset(stylesheet_parser_cache, 0, sizeof(xsltp_stylesheet_parser_cache_t));

    if (! xsltp_stylesheet_parser_cache_init(stylesheet_parser_cache)) {
        xsltp_stylesheet_parser_cache_destroy(stylesheet_parser_cache);
        return NULL;
    }

    return stylesheet_parser_cache;
}

static void
xsltp_stylesheet_parser_cache_free(xsltp_list_t *list)
{
    xsltp_list_t *el, *next_el;
    el = xsltp_list_first(list);
    while (el != xsltp_list_end(list)) {
        next_el = xsltp_list_next(el);
        xsltp_stylesheet_parser_destroy_stylesheet((xsltp_stylesheet_t *) el);
        el = next_el;
    }
}

void
xsltp_stylesheet_parser_cache_destroy(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache)
{
    if (stylesheet_parser_cache != NULL) {
#ifdef HAVE_THREADS
        xsltp_rwlock_wrlock(stylesheet_parser_cache->cache_lock);
#endif

        xsltp_stylesheet_parser_cache_free(&stylesheet_parser_cache->list);

#ifdef HAVE_THREADS
        xsltp_rwlock_unlock(stylesheet_parser_cache->cache_lock);
        xsltp_mutex_destroy(stylesheet_parser_cache->create_lock);
#endif

        xsltp_free(stylesheet_parser_cache);
    }
}

xsltp_stylesheet_t *
xsltp_stylesheet_parser_cache_lookup(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache,
    char *uri)
{
    xsltp_stylesheet_t *xsltp_stylesheet = NULL, *xsltp_stylesheet_old = NULL;
    xsltp_list_t       *el, *list;
    int                 is_found = 0;

    xsltp_log_debug1("find %s", uri);

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(stylesheet_parser_cache->cache_lock);
#endif

    list = &stylesheet_parser_cache->list;
    for (el = xsltp_list_first(list); el != xsltp_list_end(list); el = xsltp_list_next(el)) {
        xsltp_stylesheet = (xsltp_stylesheet_t *) el;

        if (strcmp(xsltp_stylesheet->uri, uri) == 0) {
            if (xsltp_stylesheet->expired == 0 && xsltp_stylesheet->error == 0) {
                xsltp_log_debug1("stylesheet %s found in cache", uri);

                is_found = 1;
                break;
            }

            if (xsltp_stylesheet->expired && xsltp_stylesheet->stylesheet != NULL) {
                xsltp_stylesheet_old = xsltp_stylesheet;
            }
        }
    }

    if (!is_found) {
        xsltp_stylesheet = xsltp_stylesheet_parser_create_stylesheet(uri);
        xsltp_list_insert_tail(list, (xsltp_list_t *) xsltp_stylesheet);
    } else if (xsltp_stylesheet->stylesheet == NULL && xsltp_stylesheet_old != NULL) {
        xsltp_log_debug1("stylesheet(expired) %s found in cache", uri);

        xsltp_stylesheet = xsltp_stylesheet_old;
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(stylesheet_parser_cache->cache_lock);
#endif

    return xsltp_stylesheet;
}

static xsltp_bool_t
xsltp_stylesheet_parser_cache_check_if_modify(xsltp_stylesheet_t *xsltp_stylesheet)
{
    xsltStylesheetPtr                stylesheet;
    xmlDocPtr                        doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;
    time_t                           cur_mtime;

    stylesheet = xsltp_stylesheet->stylesheet;
    while (stylesheet != NULL) {
        doc            = stylesheet->doc;
        doc_extra_info = doc->_private;

        xsltp_log_debug2("check stylesheet: %s extra info: %p", doc->URL, doc_extra_info);

        cur_mtime = xsltp_last_modify((const char *) doc->URL);
        if (doc_extra_info->mtime != cur_mtime) {
            xsltp_log_debug3("stylesheet is updated: %s mtime_old: %d mtime: %d", doc->URL, (int) doc_extra_info->mtime, (int) cur_mtime);

            return 1;
        }

        xsltp_log_debug0("check imports");

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
    return 0;
}

void xsltp_stylesheet_parser_cache_clean(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache, xsltp_keys_cache_t *keys_cache) {
    xsltp_stylesheet_t *xsltp_stylesheet;
    time_t              now;
    xsltp_list_t       *el, *list;

    xsltp_log_debug0("start");

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(stylesheet_parser_cache->cache_lock);
#endif

    list = &stylesheet_parser_cache->list;
    now = time(NULL);
    el = xsltp_list_first(list);
    while (el != xsltp_list_end(list)) {
        xsltp_stylesheet = (xsltp_stylesheet_t *) el;

        xsltp_log_debug1("Check stylesheet: %s", xsltp_stylesheet->uri);

        if (
            xsltp_stylesheet->expired == 0 &&
            xsltp_stylesheet_parser_cache_check_if_modify(xsltp_stylesheet)
        ) {
            xsltp_log_debug1("stylesheet %s is expired", xsltp_stylesheet->uri);

            xsltp_stylesheet->expired = now;
            xsltp_keys_cache_expire(keys_cache, xsltp_stylesheet->uri, xsltp_stylesheet->created, NULL, 0);
        }

        if (
            xsltp_stylesheet->expired != 0 &&
            (now - xsltp_stylesheet->expired) > 0
        ) {
            /* clean */
            xsltp_list_remove(el);
            el = xsltp_list_next(el);

            xsltp_log_debug1("free stylesheet %s", xsltp_stylesheet->uri);

            xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet);
        } else {
            el = xsltp_list_next(el);
        }
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(stylesheet_parser_cache->cache_lock);
#endif

    xsltp_log_debug0("end");
}

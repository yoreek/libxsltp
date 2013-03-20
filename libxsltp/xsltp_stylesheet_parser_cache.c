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
    xsltp_list_t *el;
    for (el = xsltp_list_first(list); el != xsltp_list_end(list); el = xsltp_list_next(el)) {
        xsltp_stylesheet_parser_destroy_stylesheet((xsltp_stylesheet_t *) el);
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

#ifdef WITH_DEBUG
    printf("xsltp_stylesheet_parser_cache_lookup: find %s\n", uri);
#endif

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(stylesheet_parser_cache->cache_lock);
#endif

    list = &stylesheet_parser_cache->list;
    for (el = xsltp_list_first(list); el != xsltp_list_end(list); el = xsltp_list_next(el)) {
        xsltp_stylesheet = (xsltp_stylesheet_t *) el;

        if (strcmp(xsltp_stylesheet->uri, uri) == 0) {
            if (xsltp_stylesheet->expired == 0 && xsltp_stylesheet->error == 0) {
#ifdef WITH_DEBUG
                printf("xsltp_stylesheet_parser_cache_lookup: stylesheet %s found in cache\n", uri);
#endif
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
#ifdef WITH_DEBUG
        printf("xsltp_stylesheet_parser_cache_lookup: stylesheet(expired) %s found in cache\n", uri);
#endif
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
    xsltStylesheetPtr           stylesheet;
    xmlDocPtr                   doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;

    stylesheet = xsltp_stylesheet->stylesheet;
    while (stylesheet != NULL) {
        doc            = stylesheet->doc;
        doc_extra_info = doc->_private;

        if (doc_extra_info->mtime != xsltp_last_modify(doc->URL)) {
#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_cache_check_if_modify: stylesheet is updated: %s mtime_old: %d mtime: %d\n", doc->URL, (int) doc_extra_info->mtime, (int) xsltp_last_modify(doc->URL));
#endif
            return 1;
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
    return 0;
}

/*
void xsltp_stylesheet_parser_cache_clean(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache) {
    xsltp_stylesheet_t *xsltp_stylesheet;
    time_t              now;
    xsltp_list_t       *el, *list;

#ifdef WITH_DEBUG
    printf("xsltp_stylesheet_parser_cache_clean: start\n");
#endif

    // write lock start
    xsltp_rwlock_wrlock(stylesheet_parser_cache->cache_lock);

    list = &stylesheet_parser_cache->list;
    now = time(NULL);
    el = xsltp_list_first(list);
    while (el != xsltp_list_end(list)) {
        xsltp_stylesheet = (xsltp_stylesheet_t *) el;

        if (
            xsltp_stylesheet->expired == 0 &&
            xsltp_stylesheet_parser_cache_check_if_modify(xsltp_stylesheet)
        ) {
#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_cache_clean: stylesheet %s is expired\n", xsltp_stylesheet->uri);
#endif
            xsltp_stylesheet->expired = now;
            xsltp_keys_cache_expire(xsltp_stylesheet->uri, xsltp_stylesheet->mtime, NULL, 0);
        }

        if (
            xsltp_stylesheet->expired != 0 &&
            (now - xsltp_stylesheet->expired) > 0
        ) {
            // clean
            xsltp_list_remove(el);
            el = xsltp_list_next(el);
#ifdef WITH_DEBUG
            printf("xsltp_stylesheet_parser_cache_clean: free stylesheet %s\n", xsltp_stylesheet->uri);
#endif
            xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet);
        } else {
            el = xsltp_list_next(el);
        }
    }

    // write lock finish
    xsltp_rwlock_unlock(stylesheet_parser_cache->cache_lock);

#ifdef WITH_DEBUG
    printf("xsltp_stylesheet_parser_cache_clean: end\n");
#endif
}
*/

#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

xsltp_bool_t
xsltp_document_parser_check_if_modify(xsltp_document_t *xsltp_document)
{
    xmlDocPtr                        doc;
    xsltp_xml_document_extra_info_t *doc_extra_info;

    doc            = xsltp_document->doc;
    doc_extra_info = doc->_private;

    if (doc_extra_info->mtime != xsltp_last_modify(doc->URL)) {
#ifdef WITH_DEBUG
        printf("xsltp_document_parser_check_if_modify: document is updated: %s mtime_old: %d mtime: %d\n", doc->URL, (int) doc_extra_info->mtime, (int) xsltp_last_modify(doc->URL));
#endif
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

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_cache_create: create cache\n");
#endif

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
    xsltp_list_t *el;
    for (
        el = xsltp_list_first(list);
        el != xsltp_list_end(list);
        el = xsltp_list_next(el)
    ) {
        xsltp_document_parser_destroy_document((xsltp_document_t *) el);
    }
}

void
xsltp_document_parser_cache_destroy(xsltp_document_parser_cache_t *document_parser_cache)
{
#ifdef WITH_DEBUG
    printf("xsltp_document_parser_cache_destroy: destroy cache\n");
#endif

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

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_cache_lookup: lookup document %s in cache\n", uri);
#endif

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
#ifdef WITH_DEBUG
                printf("xsltp_document_parser_cache_lookup: document %s is found in cache\n", uri);
#endif
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
#ifdef WITH_DEBUG
        printf("xsltp_document_parser_cache_lookup: document(expired) %s is found in cache\n", uri);
#endif
        xsltp_document = xsltp_document_old;
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(document_parser_cache->cache_lock);
#endif

    return xsltp_document;
}

/*
void
xsltp_document_parser_cache_clean(xsltp_document_parser_cache_t *document_parser_cache)
{
    xsltp_document_t           *xsltp_document;
    xsltp_xml_document_extra_info_t *doc_extra_info;
    xmlDocPtr                   doc;
    time_t                      now;
    xsltp_list_t               *el;

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_cache_clean: start\n");
#endif

    // write lock start
    xsltp_rwlock_wrlock(document_parser_cache->cache_lock);

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
#ifdef WITH_DEBUG
            printf("xsltp_document_parser_cache_clean: document %s is expired\n", xsltp_document->uri);
#endif
            xsltp_document->expired = now;
            xsltp_keys_cache_expire(NULL, 0, xsltp_document->uri, doc_extra_info->mtime);
        }

        if (
            xsltp_document->expired != 0 &&
            (now - xsltp_document->expired) > 0
        ) {
            // clean
            xsltp_list_remove(el);
            el = xsltp_list_next(el);
#ifdef WITH_DEBUG
            printf("xsltp_document_parser_cache_clean: free document %s\n", xsltp_document->uri);
#endif
            xsltp_document_free(xsltp_document);
        } else {
            el = xsltp_list_next(el);
        }
    }

    // write lock finish
    xsltp_rwlock_unlock(document_parser_cache->cache_lock);

#ifdef WITH_DEBUG
    printf("xsltp_document_parser_cache_clean: end\n");
#endif
}
*/

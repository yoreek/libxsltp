#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_keys_t *
xsltp_keys_cache_create_keys(char *stylesheet_uri, time_t stylesheet_mtime,
    char *document_uri, time_t document_mtime, xsltDocumentPtr xslt_document)
{
    xsltp_keys_t *xsltp_keys;

    if ((xsltp_keys = xsltp_malloc(sizeof(xsltp_keys_t))) == NULL) {
        return NULL;
    }
    memset(xsltp_keys, 0, sizeof(xsltp_keys_t));

    xsltp_keys->stylesheet_uri   = strdup(stylesheet_uri);
    xsltp_keys->stylesheet_mtime = stylesheet_mtime;
    xsltp_keys->document_uri     = strdup(document_uri);
    xsltp_keys->document_mtime   = document_mtime;

    if (xslt_document->nbKeysComputed > 0) {
        xsltp_keys->xslt_document                  = xslt_document;
    }
    else {
        xsltp_keys->xslt_document_not_computed_keys = xslt_document;
    }

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_create_keys: stylesheet %s %d, document %s %d\n",
        stylesheet_uri, (int) stylesheet_mtime,
        document_uri, (int) document_mtime
    );
#endif

    return xsltp_keys;
}

static xsltp_bool_t
xsltp_keys_cache_init(xsltp_t *processor, xsltp_keys_cache_t *keys_cache)
{
    if ((keys_cache->cache_lock = xsltp_rwlock_init()) == NULL) {
        return FALSE;
    }

    keys_cache->processor = processor;

    xsltp_list_init(&keys_cache->list);

    return TRUE;
}

xsltp_keys_cache_t *
xsltp_keys_cache_create(xsltp_t *processor)
{
    xsltp_keys_cache_t *keys_cache;

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_create: create cache\n");
#endif

    if ((keys_cache = xsltp_malloc(sizeof(xsltp_keys_cache_t))) == NULL) {
        return NULL;
    }
    memset(keys_cache, 0, sizeof(xsltp_keys_cache_t));

    if (!xsltp_keys_cache_init(processor, keys_cache)) {
        xsltp_keys_cache_destroy(keys_cache);
        return NULL;
    }

    return keys_cache;
}

static void
xsltp_keys_cache_free_keys(xsltp_keys_t *xsltp_keys, int type)
{
    if (type & XSLT_KEYS_LIST_FREE_KEYS) {
        if (xsltp_keys->xslt_document != NULL) {
#ifdef WITH_DEBUG
            printf("xsltp_keys_cache_free_keys: free document keys %p\n", xsltp_keys->xslt_document);
#endif
            xsltFreeDocumentKeys(xsltp_keys->xslt_document);
        }
        if (xsltp_keys->xslt_document_not_computed_keys != NULL) {
#ifdef WITH_DEBUG
            printf("xsltp_keys_cache_free_keys: free document keys (wo computed) %p\n", xsltp_keys->xslt_document_not_computed_keys);
#endif
            xsltFreeDocumentKeys(xsltp_keys->xslt_document_not_computed_keys);
        }
    }

    if (type & XSLT_KEYS_LIST_FREE_DATA) {
        if (xsltp_keys->xslt_document != NULL) {
            xsltp_free(xsltp_keys->xslt_document);
        }
        if (xsltp_keys->xslt_document_not_computed_keys != NULL) {
            xsltp_free(xsltp_keys->xslt_document_not_computed_keys);
        }
        xsltp_free(xsltp_keys->stylesheet_uri);
        xsltp_free(xsltp_keys->document_uri);
    }

    xsltp_free(xsltp_keys);
}

void
xsltp_keys_cache_free_keys_list(xsltp_list_t *xsltp_keys_list, int type)
{
    xsltp_list_t *el;
    xsltp_keys_t *xsltp_keys;

    for ( ;; ) {
        if (xsltp_list_empty(xsltp_keys_list))
            break;

        el = xsltp_list_last(xsltp_keys_list);
        xsltp_list_remove(el);

        xsltp_keys = (xsltp_keys_t *) el;
        xsltp_keys_cache_free_keys(xsltp_keys, type);
    }
}

void
xsltp_keys_cache_destroy(xsltp_keys_cache_t *keys_cache)
{
#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_destroy: destroy\n");
#endif
    xsltp_rwlock_wrlock(keys_cache->cache_lock);

    xsltp_keys_cache_free_keys_list(&keys_cache->list, XSLT_KEYS_LIST_FREE_KEYS | XSLT_KEYS_LIST_FREE_DATA);

    xsltp_rwlock_unlock(keys_cache->cache_lock);
    xsltp_rwlock_destroy(keys_cache->cache_lock);
}

static xsltp_keys_t *
xsltp_keys_cache_lookup(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri,
    time_t stylesheet_mtime, char *document_uri, time_t document_mtime)
{
    xsltp_keys_t *xsltp_keys = NULL;
    xsltp_list_t *el;

    for (
        el  = xsltp_list_first(&keys_cache->list);
        el != xsltp_list_end(&keys_cache->list);
        el  = xsltp_list_next(el)
    ) {
        xsltp_keys = (xsltp_keys_t *) el;

        if (strcmp(xsltp_keys->stylesheet_uri, stylesheet_uri) != 0)
            continue;
        if (xsltp_keys->stylesheet_mtime != stylesheet_mtime)
            continue;
        if (strcmp(xsltp_keys->document_uri, document_uri) != 0)
            continue;
        if (xsltp_keys->document_mtime != document_mtime)
            continue;

#ifdef WITH_DEBUG
        printf("xsltp_keys_cache_lookup: keys for document %s is found in the cache\n", document_uri);
#endif
        return xsltp_keys;
    }

    return NULL;
}

void
xsltp_keys_cache_put(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri,
    time_t stylesheet_mtime, char *document_uri, time_t document_mtime,
    xsltDocumentPtr xslt_document)
{
    xsltp_keys_t *xsltp_keys;

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_put: stylesheet '%s', document '%s'\n", stylesheet_uri, document_uri);
    printf("xsltp_keys_cache_put: xslt_document %p, xslt_document->doc %p\n", xslt_document, xslt_document->doc);
#endif

    /* write lock start */
    xsltp_rwlock_wrlock(keys_cache->cache_lock);

    xsltp_keys = xsltp_keys_cache_lookup(
        keys_cache, stylesheet_uri, stylesheet_mtime, document_uri, document_mtime
    );
    if (xsltp_keys == NULL) {
        xsltp_keys = xsltp_keys_cache_create_keys(stylesheet_uri, stylesheet_mtime, document_uri, document_mtime, xslt_document);
        if (xsltp_keys != NULL) {
            xsltp_list_insert_tail(&keys_cache->list, (xsltp_list_t *) xsltp_keys);
        }
    }
    else {
        if (xsltp_keys->xslt_document == NULL && xslt_document->nbKeysComputed > 0) {
            xsltp_keys->xslt_document = xslt_document;
        }

    }

    /* write lock finish */
    xsltp_rwlock_unlock(keys_cache->cache_lock);
}

void
xsltp_keys_cache_get(xsltp_keys_cache_t *keys_cache, xsltp_list_t *keys_list,
    char *stylesheet_uri, time_t stylesheet_mtime)
{
    xsltp_keys_t *xsltp_keys, *xsltp_keys_dup;
    xsltp_list_t *el;

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_get: stylesheet %s %d\n", stylesheet_uri, (int) stylesheet_mtime);
#endif

    xsltp_list_init(keys_list);

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_get: init keys list\n");
#endif

    /* read lock start */
    xsltp_rwlock_rdlock(keys_cache->cache_lock);

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_get: lock\n");
#endif

    for (el = xsltp_list_first(&keys_cache->list); el != xsltp_list_end(&keys_cache->list); el = xsltp_list_next(el)) {
        xsltp_keys = (xsltp_keys_t *) el;

        /*printf("s: %s\n", xsltp_keys->stylesheet_uri);*/

        if (xsltp_keys->expired != 0)
            continue;
        if (strcmp(xsltp_keys->stylesheet_uri, stylesheet_uri) != 0)
            continue;
        if (xsltp_keys->stylesheet_mtime != stylesheet_mtime)
            continue;

        if ((xsltp_keys_dup = xsltp_malloc(sizeof(xsltp_keys_t))) == NULL) {
            return;
        }
        memcpy(xsltp_keys_dup, xsltp_keys, sizeof(xsltp_keys_t));
        xsltp_list_insert_tail(keys_list, (xsltp_list_t *) xsltp_keys_dup);

#ifdef WITH_DEBUG
        printf("xsltp_keys_cache_get: keys %s for stylesheet %s is found in cache\n", xsltp_keys->document_uri, stylesheet_uri);
#endif
    }

    /* write lock finish */
    xsltp_rwlock_unlock(keys_cache->cache_lock);

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_get: done\n");
#endif
}

void
xsltp_keys_cache_expire(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri,
    time_t stylesheet_mtime, char *document_uri, time_t document_mtime)
{
    xsltp_keys_t *xsltp_keys;
    xsltp_list_t *el;
    time_t        now = time(NULL);

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_expire: stylesheet %s %d, document %p\n", stylesheet_uri, (int) stylesheet_mtime, document_uri);
#endif

    /* write lock start */
    xsltp_rwlock_wrlock(keys_cache->cache_lock);

    for (el = xsltp_list_first(&keys_cache->list); el != xsltp_list_end(&keys_cache->list); el = xsltp_list_next(el)) {
        xsltp_keys = (xsltp_keys_t *) el;

#ifdef WITH_DEBUG
        printf("xsltp_keys_cache_expire: search stylesheet %s %d, document %s\n", xsltp_keys->stylesheet_uri, (int) xsltp_keys->stylesheet_mtime, xsltp_keys->document_uri);
#endif

        if (xsltp_keys->expired != 0)
            continue;
        if (stylesheet_uri && strcmp(xsltp_keys->stylesheet_uri, stylesheet_uri) != 0)
            continue;
        if (stylesheet_uri && xsltp_keys->stylesheet_mtime != stylesheet_mtime)
            continue;
        if (document_uri && strcmp(xsltp_keys->document_uri, document_uri) != 0)
            continue;
        if (document_uri && xsltp_keys->document_mtime != document_mtime)
            continue;

        xsltp_keys->expired = now;

#ifdef WITH_DEBUG
        printf("xsltp_keys_cache_expire: expired stylesheet %s, document %s\n", xsltp_keys->stylesheet_uri, xsltp_keys->document_uri);
#endif
    }

    /* write lock finish */
    xsltp_rwlock_unlock(keys_cache->cache_lock);
}

/*
void xsltp_keys_cache_clean(xsltp_keys_cache_t *keys_cache) {
    xsltp_keys_t *xsltp_keys;
    xsltp_list_t *el;
    time_t        now;

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_clean: start\n");
#endif

    // write lock start
    xsltp_rwlock_wrlock(keys_cache->cache_lock);

    now = time(NULL);
    el = xsltp_list_first(&keys_cache->list);
    while (el != xsltp_list_end(&keys_cache->list)) {
        xsltp_keys = (xsltp_keys_t *) el;

        if (
            xsltp_keys->expired != 0 &&
            (now - xsltp_keys->expired) > 0
        ) {
            // clean
            xsltp_list_remove(el);
            el = xsltp_list_next(el);
#ifdef WITH_DEBUG
            printf("xsltp_keys_cache_clean: free keys for document %s\n", xsltp_keys->document_uri);
#endif
            xsltp_keys_cache_free_keys(xsltp_keys, XSLT_KEYS_LIST_FREE_KEYS | XSLT_KEYS_LIST_FREE_DATA);
        } else {
            el = xsltp_list_next(el);
        }
    }

    // write lock finish
    xsltp_rwlock_unlock(keys_cache->cache_lock);

#ifdef WITH_DEBUG
    printf("xsltp_keys_cache_clean: end\n");
#endif
}
*/

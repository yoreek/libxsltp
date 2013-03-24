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

    xsltp_log_debug4("stylesheet %s %d, document %s %d",
        stylesheet_uri, (int) stylesheet_mtime,
        document_uri, (int) document_mtime
    );

    return xsltp_keys;
}

static xsltp_bool_t
xsltp_keys_cache_init(xsltp_t *processor, xsltp_keys_cache_t *keys_cache)
{
#ifdef HAVE_THREADS
    if ((keys_cache->cache_lock = xsltp_rwlock_init()) == NULL) {
        return FALSE;
    }
#endif

    keys_cache->processor = processor;

    xsltp_list_init(&keys_cache->list);

    return TRUE;
}

xsltp_keys_cache_t *
xsltp_keys_cache_create(xsltp_t *processor)
{
    xsltp_keys_cache_t *keys_cache;

    xsltp_log_debug0("create cache");

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
            xsltp_log_debug1("free document keys %p", xsltp_keys->xslt_document);

            xsltFreeDocumentKeys(xsltp_keys->xslt_document);
        }
        if (xsltp_keys->xslt_document_not_computed_keys != NULL) {
            xsltp_log_debug1("free document keys (wo computed) %p", xsltp_keys->xslt_document_not_computed_keys);

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
    xsltp_log_debug0("destroy");

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(keys_cache->cache_lock);
#endif

    xsltp_keys_cache_free_keys_list(&keys_cache->list, XSLT_KEYS_LIST_FREE_KEYS | XSLT_KEYS_LIST_FREE_DATA);

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(keys_cache->cache_lock);
    xsltp_rwlock_destroy(keys_cache->cache_lock);
#endif
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

        xsltp_log_debug1("keys for document %s is found in the cache", document_uri);

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

    xsltp_log_debug2("stylesheet '%s', document '%s'", stylesheet_uri, document_uri);
    xsltp_log_debug2("xslt_document %p, xslt_document->doc %p", xslt_document, xslt_document->doc);

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(keys_cache->cache_lock);
#endif

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

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(keys_cache->cache_lock);
#endif
}

void
xsltp_keys_cache_get(xsltp_keys_cache_t *keys_cache, xsltp_list_t *keys_list,
    char *stylesheet_uri, time_t stylesheet_mtime)
{
    xsltp_keys_t *xsltp_keys, *xsltp_keys_dup;
    xsltp_list_t *el;

    xsltp_log_debug2("stylesheet %s %d", stylesheet_uri, (int) stylesheet_mtime);

    xsltp_list_init(keys_list);

    xsltp_log_debug0("init keys list");

#ifdef HAVE_THREADS
    xsltp_rwlock_rdlock(keys_cache->cache_lock);
#endif

    xsltp_log_debug0("lock");

    for (el = xsltp_list_first(&keys_cache->list); el != xsltp_list_end(&keys_cache->list); el = xsltp_list_next(el)) {
        xsltp_keys = (xsltp_keys_t *) el;

        /*printf("s: %s", xsltp_keys->stylesheet_uri);*/

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

        xsltp_log_debug2("keys %s for stylesheet %s is found in cache", xsltp_keys->document_uri, stylesheet_uri);
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(keys_cache->cache_lock);
#endif

    xsltp_log_debug0("done");
}

void
xsltp_keys_cache_expire(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri,
    time_t stylesheet_mtime, char *document_uri, time_t document_mtime)
{
    xsltp_keys_t *xsltp_keys;
    xsltp_list_t *el;
    time_t        now = time(NULL);

    xsltp_log_debug3("stylesheet %s %d, document %p", stylesheet_uri, (int) stylesheet_mtime, document_uri);

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(keys_cache->cache_lock);
#endif

    for (el = xsltp_list_first(&keys_cache->list); el != xsltp_list_end(&keys_cache->list); el = xsltp_list_next(el)) {
        xsltp_keys = (xsltp_keys_t *) el;

        xsltp_log_debug3("search stylesheet %s %d, document %s", xsltp_keys->stylesheet_uri, (int) xsltp_keys->stylesheet_mtime, xsltp_keys->document_uri);

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

        xsltp_log_debug2("expired stylesheet %s, document %s", xsltp_keys->stylesheet_uri, xsltp_keys->document_uri);
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(keys_cache->cache_lock);
#endif
}

void xsltp_keys_cache_clean(xsltp_keys_cache_t *keys_cache) {
    xsltp_keys_t *xsltp_keys;
    xsltp_list_t *el;
    time_t        now;

    xsltp_log_debug0("start");

#ifdef HAVE_THREADS
    xsltp_rwlock_wrlock(keys_cache->cache_lock);
#endif

    now = time(NULL);
    el = xsltp_list_first(&keys_cache->list);
    while (el != xsltp_list_end(&keys_cache->list)) {
        xsltp_keys = (xsltp_keys_t *) el;

        if (
            xsltp_keys->expired != 0 &&
            (now - xsltp_keys->expired) > 0
        ) {
            /* clean */
            xsltp_list_remove(el);
            el = xsltp_list_next(el);
            xsltp_log_debug1("free keys for document %s", xsltp_keys->document_uri);

            xsltp_keys_cache_free_keys(xsltp_keys, XSLT_KEYS_LIST_FREE_KEYS | XSLT_KEYS_LIST_FREE_DATA);
        } else {
            el = xsltp_list_next(el);
        }
    }

#ifdef HAVE_THREADS
    xsltp_rwlock_unlock(keys_cache->cache_lock);
#endif

    xsltp_log_debug0("end");
}

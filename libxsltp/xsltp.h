#ifndef _XSLTP_H_INCLUDED_
#define _XSLTP_H_INCLUDED_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/documents.h>
#include <libxslt/imports.h>
#include <libxslt/xsltutils.h>
#include <libxslt/keys.h>
#include <libxslt/extensions.h>
#ifdef HAVE_LIBEXSLT
#include <libexslt/exslt.h>
#endif
#ifdef HAVE_GLIB
#include <glib.h>
#endif

#define XSLTP_PROCESSOR_ID              (0x7755)
#define XSLTP_MAX_TRANSFORMATIONS        16
#define XSLTP_MAX_TRANSFORMATIONS_PARAMS 254
#define XSLTP_EXTENSION_COMMON_NS        "http://xsltproc.org/xslt/common"
#define XSLTP_EXTENSION_STRING_NS        "http://xsltproc.org/xslt/string"

#ifdef HAVE_ICU
typedef UChar32 (*xsltp_extension_case_convert_func_t) (UChar32 c);
#endif

typedef struct xsltp_transform_ctxt xsltp_transform_ctxt_t;
struct xsltp_transform_ctxt {
    char *stylesheet;
    char *params[XSLTP_MAX_TRANSFORMATIONS_PARAMS + 1];
};

typedef struct xsltp_stylesheet xsltp_stylesheet_t;
struct xsltp_stylesheet {
    xsltp_list_t                     list;
    char                            *uri;
    xsltStylesheetPtr                stylesheet;
    xsltDocumentPtr                  doc_list;
    time_t                           expired;
    time_t                           created;
    int                              creating;
    int                              error;
};

typedef struct xsltp_document xsltp_document_t;
struct xsltp_document {
    xsltp_list_t                     list;
    char                            *uri;
    xmlDocPtr                        doc;
    time_t                           expired;
    int                              creating;
    int                              error;
};

typedef struct xsltp_xml_document_extra_info xsltp_xml_document_extra_info_t;
struct xsltp_xml_document_extra_info {
    time_t                           mtime;
};

typedef struct xsltp_keys xsltp_keys_t;
struct xsltp_keys {
    xsltp_list_t                     list;
    char                            *stylesheet_uri;
    time_t                           stylesheet_mtime;
    char                            *document_uri;
    time_t                           document_mtime;
    xsltDocumentPtr                  xslt_document;
    xsltDocumentPtr                  xslt_document_not_computed_keys;
    time_t                           expired;
};

typedef struct _xsltp_t xsltp_t;
typedef struct xsltp_profiler xsltp_profiler_t;

typedef struct {
#ifdef HAVE_THREADS
    xsltp_rwlock_t                  *cache_lock;
#endif
    xsltp_list_t                     list;
    xsltp_t                         *processor;
} xsltp_keys_cache_t;

typedef struct {
#ifdef HAVE_THREADS
    xsltp_rwlock_t                  *cache_lock;
    xsltp_mutex_t                   *create_lock;
#endif
    xsltp_list_t                     list;
} xsltp_stylesheet_parser_cache_t;

typedef struct {
#ifdef HAVE_THREADS
    xsltp_mutex_t                   *parse_lock;
    xsltp_cond_t                    *parse_cond;
#endif
    xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache;
    xsltp_t                         *processor;
} xsltp_stylesheet_parser_t;

typedef struct {
#ifdef HAVE_THREADS
    xsltp_rwlock_t                  *cache_lock;
    xsltp_mutex_t                   *create_lock;
#endif
    xsltp_list_t                     list;
} xsltp_document_parser_cache_t;

typedef struct {
#ifdef HAVE_THREADS
    xsltp_mutex_t                   *parse_lock;
    xsltp_cond_t                    *parse_cond;
#endif
    xsltp_document_parser_cache_t   *cache;
    xsltp_t                         *processor;
} xsltp_document_parser_t;

struct _xsltp_t {
    u_int32_t                        id;
    u_int32_t                        stylesheet_max_depth;
    xsltp_bool_t                     stylesheet_caching_enable;
    xsltp_bool_t                     document_caching_enable;
    xsltp_bool_t                     keys_caching_enable;
    xsltp_bool_t                     profiler_enable;
    char                            *profiler_stylesheet;
    u_int8_t                         profiler_repeat;
    xsltp_stylesheet_parser_t       *stylesheet_parser;
    xsltp_document_parser_t         *document_parser;
    xsltp_keys_cache_t              *keys_cache;
    xsltp_profiler_t                *profiler;
};

struct xsltp_profiler {
    u_int8_t                         repeat;
    xsltStylesheetPtr                stylesheet;
    xsltp_t                         *processor;
    FILE                            *handle;
};

typedef struct xsltp_profiler_result xsltp_profiler_result_t;
struct xsltp_profiler_result {
    xmlDocPtr                        doc;
};

typedef struct {
    xsltp_t                         *processor;
    xsltp_stylesheet_t              *xsltp_stylesheet;
    xmlDocPtr                        doc;
    xsltp_profiler_result_t         *profiler_result;
} xsltp_result_t;

xsltp_t *xsltp_create(void);
void xsltp_destroy(xsltp_t *processor);
xsltp_result_t *xsltp_transform(xsltp_t *processor,
    char *stylesheet_uri, xmlDocPtr doc, const char **params, xsltp_profiler_result_t *profiler_result);
xsltp_result_t *xsltp_transform_multi(xsltp_t *processor, xsltp_transform_ctxt_t *transform_ctxt, xmlDocPtr doc);
void xsltp_global_init(void);
void xsltp_global_cleanup(void);

xsltp_result_t *xsltp_result_create(xsltp_t *processor);
void xsltp_result_destroy(xsltp_result_t *result);
int xsltp_result_save(xsltp_result_t *result, xmlOutputBufferPtr output);
int xsltp_result_save_to_file(xsltp_result_t *result, char *filename);
int xsltp_result_save_to_string(xsltp_result_t *result, char **buf, int *buf_len);

xsltp_profiler_result_t *xsltp_profiler_result_create(xsltp_t *processor);
void xsltp_profiler_result_destroy(xsltp_profiler_result_t *profiler_result);
xsltp_bool_t xsltp_profiler_result_apply(xsltp_profiler_t *profiler, xsltp_profiler_result_t *profiler_result, xmlDocPtr doc);
void xsltp_profiler_result_update(xsltp_profiler_result_t *profiler_result,
    xsltp_stylesheet_t *xsltp_stylesheet, const char **params,
    long spent, xmlDocPtr doc, xmlDocPtr profile_info);
xsltp_profiler_t *xsltp_profiler_create(xsltp_t *processor);
void xsltp_profiler_destroy(xsltp_profiler_t *profiler);

xsltp_stylesheet_parser_t *xsltp_stylesheet_parser_create(xsltp_t *processor);
void xsltp_stylesheet_parser_destroy(xsltp_stylesheet_parser_t *stylesheet_parser);
xsltp_stylesheet_t *xsltp_stylesheet_parser_create_stylesheet(char *uri);
void xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet_t *xsltp_stylesheet);
int xsltp_stylesheet_parser_stylesheet_is_updated(xsltp_stylesheet_t *xsltp_stylesheet);
xsltp_stylesheet_t *xsltp_stylesheet_parser_parse_file(xsltp_stylesheet_parser_t *stylesheet_parser, char *uri);
void xsltp_stylesheet_parser_destroy_extra_info(xsltp_stylesheet_t *xsltp_stylesheet);
xsltp_bool_t xsltp_stylesheet_parser_create_extra_info(xsltp_stylesheet_t *xsltp_stylesheet);

xsltp_stylesheet_parser_cache_t *xsltp_stylesheet_parser_cache_create(void);
void xsltp_stylesheet_parser_cache_destroy(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache);
xsltp_stylesheet_t *xsltp_stylesheet_parser_cache_lookup(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache, char *uri);
void xsltp_stylesheet_parser_cache_clean(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache, xsltp_keys_cache_t *keys_cache);

xsltp_document_parser_t *xsltp_document_parser_create(xsltp_t *processor);
void xsltp_document_parser_destroy(xsltp_document_parser_t *document_parser);
xsltp_document_t *xsltp_document_parser_create_document(char *uri);
void xsltp_document_parser_destroy_document(xsltp_document_t *xsltp_document);

xsltp_document_parser_cache_t *xsltp_document_parser_cache_create(void);
void xsltp_document_parser_cache_destroy(xsltp_document_parser_cache_t *cache);
xsltp_document_t *xsltp_document_parser_cache_lookup(xsltp_document_parser_cache_t *cache, char *uri);
void xsltp_document_parser_cache_clean(xsltp_document_parser_cache_t *cache, xsltp_keys_cache_t *keys_cache);

int xsltp_extension_init(void);

#define XSLT_KEYS_LIST_FREE_NONE 0
#define XSLT_KEYS_LIST_FREE_DATA 1
#define XSLT_KEYS_LIST_FREE_KEYS 2

xsltp_keys_cache_t *xsltp_keys_cache_create(xsltp_t *processor);
void xsltp_keys_cache_destroy(xsltp_keys_cache_t *keys_cache);
void xsltp_keys_cache_put(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri, time_t stylesheet_mtime,
    char *document_uri, time_t document_mtime, xsltDocumentPtr xslt_document);
void xsltp_keys_cache_get(xsltp_keys_cache_t *keys_cache, xsltp_list_t *keys_list,
    char *stylesheet_uri, time_t stylesheet_mtime);
void xsltp_keys_cache_expire(xsltp_keys_cache_t *keys_cache, char *stylesheet_uri, time_t stylesheet_mtime,
    char *document_uri, time_t document_mtime);
void xsltp_keys_cache_clean(xsltp_keys_cache_t *keys_cache);
void xsltp_keys_cache_free_keys_list(xsltp_list_t *xsltp_keys_list, int type);

#endif /* _XSLTP_H_INCLUDED_ */

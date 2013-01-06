#ifndef _XSLTP_H_INCLUDED_
#define _XSLTP_H_INCLUDED_

/*#include <libxml/globals.h>*/
/*#include <libxml/threads.h>*/

#include <libxml/parser.h>
#include <libxml/xmlsave.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/documents.h>
#include <libexslt/exslt.h>

typedef struct xsltp_stylesheet xsltp_stylesheet_t;
struct xsltp_stylesheet {
    xsltp_list_t            list;
    char                   *uri;
    xsltStylesheetPtr       stylesheet;
    xsltDocumentPtr         doc_list;
    time_t                  expired;
    time_t                  mtime;
    int                     creating;
    int                     error;
};

typedef struct xsltp_document xsltp_document_t;
struct xsltp_document {
    xsltp_list_t      list;
    char             *uri;
    xmlDocPtr         doc;
    time_t            expired;
    int               creating;
    int               error;
};

typedef struct xsltp_xml_document_extra_info xsltp_xml_document_extra_info_t;
struct xsltp_xml_document_extra_info {
    time_t mtime;
};

typedef struct xsltp_keys xsltp_keys_t;
struct xsltp_keys {
    xsltp_list_t      list;
    char             *stylesheet_uri;
    time_t            stylesheet_mtime;
    char             *document_uri;
    time_t            document_mtime;
    xsltDocumentPtr   xslt_document;
    xsltDocumentPtr   xslt_document_not_computed_keys;
    time_t            expired;
};

typedef struct _xsltp_t xsltp_t;

typedef struct {
    xsltp_rwlock_t                  *cache_lock;
    xsltp_list_t                     list;
    xsltp_t                         *processor;
} xsltp_keys_cache_t;

typedef struct {
    xsltp_rwlock_t                  *cache_lock;
    xsltp_mutex_t                   *create_lock;
    xsltp_list_t                     list;
} xsltp_stylesheet_parser_cache_t;

typedef struct {
    xsltp_mutex_t                   *parse_lock;
    xsltp_cond_t                    *parse_cond;
    xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache;
    xsltp_t                         *processor;
} xsltp_stylesheet_parser_t;

typedef struct {
    xsltp_rwlock_t                  *cache_lock;
    xsltp_mutex_t                   *create_lock;
    xsltp_list_t                     list;
} xsltp_document_parser_cache_t;

typedef struct {
    xsltp_mutex_t                   *parse_lock;
    xsltp_cond_t                    *parse_cond;
    xsltp_document_parser_cache_t   *cache;
    xsltp_t                         *processor;
} xsltp_document_parser_t;

struct _xsltp_t {
    xsltp_bool_t                     stylesheet_cache_enable;
    xsltp_bool_t                     document_cache_enable;
    xsltp_stylesheet_parser_t       *stylesheet_parser;
    xsltp_document_parser_t         *document_parser;
    xsltp_keys_cache_t              *keys_cache;
};

typedef struct {
    xsltp_t                         *processor;
    xsltp_stylesheet_t              *xsltp_stylesheet;
    xmlDocPtr                        doc;
} xsltp_result_t;

xsltp_t *xsltp_create(xsltp_bool_t stylesheet_cache_enable, xsltp_bool_t document_cache_enable);
void xsltp_destroy(xsltp_t *processor);
xsltp_result_t *xsltp_transform(xsltp_t *processor,
    char *stylesheet_uri, xmlDocPtr doc, const char **params);

xsltp_result_t *xsltp_result_create(xsltp_t *processor);
void xsltp_result_destroy(xsltp_result_t *result);
int xsltp_result_save(xsltp_result_t *result, xmlOutputBufferPtr output);
int xsltp_result_save_to_file(xsltp_result_t *result, char *filename);
int xsltp_result_save_to_string(xsltp_result_t *result, char **buf, int *buf_len);

xsltp_stylesheet_parser_t *xsltp_stylesheet_parser_create(xsltp_t *processor);
void xsltp_stylesheet_parser_destroy(xsltp_stylesheet_parser_t *stylesheet_parser);
xsltp_stylesheet_t *xsltp_stylesheet_parser_create_stylesheet(char *uri);
void xsltp_stylesheet_parser_destroy_stylesheet(xsltp_stylesheet_t *xsltp_stylesheet);
int xsltp_stylesheet_parser_stylesheet_is_updated(xsltp_stylesheet_t *xsltp_stylesheet);
xsltp_stylesheet_t *xsltp_stylesheet_parser_parse_file(xsltp_stylesheet_parser_t *stylesheet_parser, char *uri);

xsltp_stylesheet_parser_cache_t *xsltp_stylesheet_parser_cache_create(void);
void xsltp_stylesheet_parser_cache_destroy(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache);
xsltp_stylesheet_t *xsltp_stylesheet_parser_cache_lookup(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache, char *uri);
void xsltp_stylesheet_parser_cache_clean(xsltp_stylesheet_parser_cache_t *stylesheet_parser_cache);

xsltp_document_parser_t *xsltp_document_parser_create(xsltp_t *processor);
void xsltp_document_parser_destroy(xsltp_document_parser_t *document_parser);
xsltp_document_t *xsltp_document_parser_create_document(char *uri);
void xsltp_document_parser_destroy_document(xsltp_document_t *xsltp_document);

xsltp_document_parser_cache_t *xsltp_document_parser_cache_create(void);
void xsltp_document_parser_cache_destroy(xsltp_document_parser_cache_t *cache);
xsltp_document_t *xsltp_document_parser_cache_lookup(xsltp_document_parser_cache_t *cache, char *uri);
void xsltp_document_parser_cache_clean(xsltp_document_parser_cache_t *cache);

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

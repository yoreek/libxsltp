#include "xsltp_config.h"
#include "xsltp_core.h"

#ifndef _XSLTP_LIST_INCLUDED_
#define _XSLTP_LIST_INCLUDED_

typedef struct xsltp_list xsltp_list_t;
struct xsltp_list {
    xsltp_list_t *prev;
    xsltp_list_t *next;
};

#define xsltp_list_init(l)                                                      \
    (l)->prev = l;                                                              \
    (l)->next = l

#define xsltp_list_empty(l)                                                     \
    (l == (l)->prev)

#define xsltp_list_insert_head(l, x)                                            \
    (x)->next = (l)->next;                                                      \
    (x)->next->prev = x;                                                        \
    (x)->prev = l;                                                              \
    (l)->next = x

#define xsltp_list_insert_after   xsltp_list_insert_head

#define xsltp_list_insert_tail(l, x)                                            \
    (x)->prev = (l)->prev;                                                      \
    (x)->prev->next = x;                                                        \
    (x)->next = l;                                                              \
    (l)->prev = x

#define xsltp_list_first(l)                                                     \
    (l)->next

#define xsltp_list_last(l)                                                      \
    (l)->prev

#define xsltp_list_end(l)                                                       \
    (l)

#define xsltp_list_next(l)                                                      \
    (l)->next

#define xsltp_list_prev(l)                                                      \
    (l)->prev

#define xsltp_list_remove(x)                                                    \
    (x)->next->prev = (x)->prev;                                                \
    (x)->prev->next = (x)->next

#define xsltp_list_split(h, q, n)                                               \
    (n)->prev = (h)->prev;                                                      \
    (n)->prev->next = n;                                                        \
    (n)->next = q;                                                              \
    (h)->prev = (q)->prev;                                                      \
    (h)->prev->next = h;                                                        \
    (q)->prev = n

/* Merge two list */
#define xsltp_list_merge(a, b)                                                  \
    (a)->prev->next = (b)->next;                                                \
    (b)->next->prev = (a)->prev;                                                \
    (a)->prev = (b)->prev;                                                      \
    (a)->prev->next = a

void xsltp_list_sort(xsltp_list_t *list, int (*cmp)(const xsltp_list_t *, const xsltp_list_t *));

#endif /* _XSLTP_LIST_H_INCLUDED_ */

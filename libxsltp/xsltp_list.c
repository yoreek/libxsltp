#include "xsltp_config.h"
#include "xsltp_core.h"

void xsltp_list_sort(xsltp_list_t *list, int (*cmp)(const xsltp_list_t *, const xsltp_list_t *)) {
    xsltp_list_t *el, *prev, *next;

    el = xsltp_list_first(list);

    if (el == xsltp_list_last(list)) {
        return;
    }

    for (el = xsltp_list_next(el); el != xsltp_list_end(list); el = next) {

        prev = xsltp_list_prev(el);
        next = xsltp_list_next(el);

        xsltp_list_remove(el);

        do {
            if (cmp(prev, el) <= 0) {
                break;
            }

            prev = xsltp_list_prev(prev);

        } while (prev != xsltp_list_end(list));

        xsltp_list_insert_after(prev, el);
    }
}

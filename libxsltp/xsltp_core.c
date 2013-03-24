#include "xsltp_config.h"
#include "xsltp_core.h"

void *xsltp_malloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
        perror("malloc failed");
    return p;
}

void xsltp_free(void *p) {
    free(p);
}

time_t xsltp_last_modify(const char *file_name) {
    struct stat sb;

    xsltp_log_debug1("get last modify time for file: %s", file_name);

    if (stat(file_name, &sb) == -1) {
        xsltp_log_debug0("can't detect last modify time");
        return 0;
    }

    xsltp_log_debug1("last modify time: %d", sb.st_mtime);

    return sb.st_mtime;
}

#include "xsltp_config.h"
#include "xsltp_core.h"

void
xsltp_log_debug(const char *func, int line, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) fprintf(stderr, "(DEBUG) %s[%d]: ", func, line);
    (void) vfprintf(stderr, fmt, args);
    (void) fprintf(stderr, "\n");
    va_end(args);
}

void
xsltp_log_error(const char *func, int line, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) fprintf(stderr, "(ERROR) %s[%d] - ", func, line);
    (void) vfprintf(stderr, fmt, args);
    (void) fprintf(stderr, "\n");
    va_end(args);
}

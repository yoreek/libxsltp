#include "xsltp_config.h"
#include "xsltp_core.h"

#define XSLTP_LOG_MSG_LEN 512

static xsltp_log_error_handler_t  xsltp_log_generic_error = xsltp_log_default_error_handler;
static void                      *xsltp_log_error_ctxt    = NULL;

void
xsltp_log_init(void *ctxt, xsltp_log_error_handler_t handler)
{
    xsltp_log_error_ctxt = ctxt;

    if (handler != NULL) {
        xsltp_log_generic_error = handler;
    }
    else {
        xsltp_log_generic_error = xsltp_log_default_error_handler;
    }

    xmlSetGenericErrorFunc(NULL, xsltp_log_xml_error_handler);
    xsltSetGenericErrorFunc(NULL, xsltp_log_xslt_error_handler);
}

void
xsltp_log_set_error_handler(void *ctxt, xsltp_log_error_handler_t handler)
{
    xsltp_log_error_ctxt = ctxt;

    if (handler != NULL) {
        xsltp_log_generic_error = handler;
    }
    else {
        xsltp_log_generic_error = xsltp_log_default_error_handler;
    }
}

void
xsltp_log_default_error_handler(void *ctxt, int log_level, const char *func, int line, const char *msg, ...)
{
    va_list args;

    switch (log_level) {
        case XSLTP_LOG_DEBUG:
            (void) fprintf(stderr, "(DEBUG) %s[%.0d]: ", func, line);
            break;
        case XSLTP_LOG_ERROR:
            (void) fprintf(stderr, "(ERROR) %s[%.0d]: ", func, line);
            break;
    }

    va_start(args, msg);
    (void) vfprintf(stderr, msg, args);
    if (msg[strlen(msg) - 1] != '\n') {
        (void) fprintf(stderr, "\n");
    }
    va_end(args);
}

void
xsltp_log_xml_error_handler(void *ctxt, const char *msg, ...)
{
    va_list args;
    char    tmp[XSLTP_LOG_MSG_LEN];

    va_start(args, msg);
    (void) vsnprintf(tmp, sizeof(tmp), msg, args);
    va_end(args);

    xsltp_log_generic_error(xsltp_log_error_ctxt, XSLTP_LOG_ERROR, "xml", 0, tmp, NULL);
}

void
xsltp_log_xslt_error_handler(void *ctxt, const char *msg, ...)
{
    va_list args;
    char    tmp[XSLTP_LOG_MSG_LEN];

    va_start(args, msg);
    (void) vsnprintf(tmp, sizeof(tmp), msg, args);
    va_end(args);

    xsltp_log_generic_error(xsltp_log_error_ctxt, XSLTP_LOG_ERROR, "xslt", 0, tmp, NULL);
}

void
xsltp_log_debug(const char *func, int line, const char *msg, ...)
{
    va_list args;
    char    tmp[XSLTP_LOG_MSG_LEN];

    va_start(args, msg);
    (void) vsnprintf(tmp, sizeof(tmp), msg, args);
    va_end(args);

    xsltp_log_generic_error(xsltp_log_error_ctxt, XSLTP_LOG_DEBUG, func, line, tmp, NULL);
}

void
xsltp_log_error(const char *func, int line, const char *msg, ...)
{
    va_list args;
    char    tmp[XSLTP_LOG_MSG_LEN];

    va_start(args, msg);
    (void) vsnprintf(tmp, sizeof(tmp), msg, args);
    va_end(args);

    xsltp_log_generic_error(xsltp_log_error_ctxt, XSLTP_LOG_ERROR, func, line, tmp, NULL);
}

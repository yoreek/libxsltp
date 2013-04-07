#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

#ifndef _XSLTP_LOG_INCLUDED_
#define _XSLTP_LOG_INCLUDED_

#define XSLTP_LOG_ERROR 0
#define XSLTP_LOG_DEBUG 1

#ifdef WITH_DEBUG
#define xsltp_log_debug0(msg)                                                 \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg)
#define xsltp_log_debug1(msg, arg1)                                           \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1)
#define xsltp_log_debug2(msg, arg1, arg2)                                     \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2)
#define xsltp_log_debug3(msg, arg1, arg2, arg3)                               \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3)
#define xsltp_log_debug4(msg, arg1, arg2, arg3, arg4)                         \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4)
#define xsltp_log_debug5(msg, arg1, arg2, arg3, arg4, arg5)                   \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_debug6(msg, arg1, arg2, arg3, arg4, arg5, arg6)             \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_debug7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)       \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_debug8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        xsltp_log_debug(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#else
#define xsltp_log_debug0(msg)
#define xsltp_log_debug1(msg, arg1)
#define xsltp_log_debug2(msg, arg1, arg2)
#define xsltp_log_debug3(msg, arg1, arg2, arg3)
#define xsltp_log_debug4(msg, arg1, arg2, arg3, arg4)
#define xsltp_log_debug5(msg, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_debug6(msg, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_debug7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_debug8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#endif

#define xsltp_log_error0(msg)                                                 \
        xsltp_log_error(__FUNCTION__, __LINE__, msg)
#define xsltp_log_error1(msg, arg1)                                           \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1)
#define xsltp_log_error2(msg, arg1, arg2)                                     \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2)
#define xsltp_log_error3(msg, arg1, arg2, arg3)                               \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3)
#define xsltp_log_error4(msg, arg1, arg2, arg3, arg4)                         \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4)
#define xsltp_log_error5(msg, arg1, arg2, arg3, arg4, arg5)                   \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_error6(msg, arg1, arg2, arg3, arg4, arg5, arg6)             \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_error7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)       \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_error8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        xsltp_log_error(__FUNCTION__, __LINE__, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

typedef void (*xsltp_log_error_handler_t) (void *ctxt, int log_level, const char *func, int line, const char *msg, ...);

void xsltp_log_init(void *ctxt, xsltp_log_error_handler_t handler);
void xsltp_log_set_error_handler(void *ctxt, xsltp_log_error_handler_t handler);
void xsltp_log_default_error_handler(void *ctxt, int log_level, const char *func, int line, const char *msg, ...);
void xsltp_log_debug(const char *func, int line, const char *msg, ...);
void xsltp_log_error(const char *func, int line, const char *msg, ...);
void xsltp_log_xml_error_handler(void *ctxt, const char *msg, ...);
void xsltp_log_xslt_error_handler(void *ctxt, const char *msg, ...);

#endif /* _XSLTP_LOG_H_INCLUDED_ */

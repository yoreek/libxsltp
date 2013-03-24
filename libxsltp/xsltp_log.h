#include "xsltp_config.h"
#include "xsltp_core.h"

#ifndef _XSLTP_LOG_INCLUDED_
#define _XSLTP_LOG_INCLUDED_

#ifdef WITH_DEBUG
#define xsltp_log_debug0(fmt)                                                 \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt)
#define xsltp_log_debug1(fmt, arg1)                                           \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1)
#define xsltp_log_debug2(fmt, arg1, arg2)                                     \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2)
#define xsltp_log_debug3(fmt, arg1, arg2, arg3)                               \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3)
#define xsltp_log_debug4(fmt, arg1, arg2, arg3, arg4)                         \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4)
#define xsltp_log_debug5(fmt, arg1, arg2, arg3, arg4, arg5)                   \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_debug6(fmt, arg1, arg2, arg3, arg4, arg5, arg6)             \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_debug7(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)       \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_debug8(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        xsltp_log_debug(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#else
#define xsltp_log_debug0(fmt)
#define xsltp_log_debug1(fmt, arg1)
#define xsltp_log_debug2(fmt, arg1, arg2)
#define xsltp_log_debug3(fmt, arg1, arg2, arg3)
#define xsltp_log_debug4(fmt, arg1, arg2, arg3, arg4)
#define xsltp_log_debug5(fmt, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_debug6(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_debug7(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_debug8(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#endif

#define xsltp_log_error0(fmt)                                                 \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt)
#define xsltp_log_error1(fmt, arg1)                                           \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1)
#define xsltp_log_error2(fmt, arg1, arg2)                                     \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2)
#define xsltp_log_error3(fmt, arg1, arg2, arg3)                               \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3)
#define xsltp_log_error4(fmt, arg1, arg2, arg3, arg4)                         \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4)
#define xsltp_log_error5(fmt, arg1, arg2, arg3, arg4, arg5)                   \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5)
#define xsltp_log_error6(fmt, arg1, arg2, arg3, arg4, arg5, arg6)             \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
#define xsltp_log_error7(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)       \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define xsltp_log_error8(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        xsltp_log_error(__FUNCTION__, __LINE__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

void xsltp_log_debug(const char *func, int line, const char *fmt, ...);
void xsltp_log_error(const char *func, int line, const char *fmt, ...);

#endif /* _XSLTP_LOG_H_INCLUDED_ */

#ifndef _XSLTP_CORE_H_INCLUDED_
#define _XSLTP_CORE_H_INCLUDED_

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/*
 * This is a workaround for one version of the HP C compiler
 * (c89 on HP-UX 9.04, also Stratus FTX), which will dump core
 * if given 'long double' for varargs.
 */
#ifdef HAVE_VA_ARG_LONG_DOUBLE_BUG
#define LONG_DOUBLE double
#else
#define LONG_DOUBLE long double
#endif

typedef uintptr_t xsltp_bool_t;
void *xsltp_malloc(size_t size);
void xsltp_free(void *p);
time_t xsltp_last_modify(const char *file_name);

#ifdef HAVE_OPENSSL
#include "xsltp_md5.h"
#endif

#include "xsltp_string.h"
#include "xsltp_list.h"
#include "xsltp_log.h"

#ifdef HAVE_THREADS
#include "xsltp_thread.h"
#endif

#endif /* _XSLTP_CORE_H_INCLUDED_ */

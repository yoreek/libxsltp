AC_PREREQ([2.59])
AC_INIT([XSLT processing library], [0.04], [yoreek@yahoo.com],
        [libxsltp], [https://github.com/yoreek/libxsltp/])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS

LIBXSLT_REQUIRED_VERSION=1.1.25
AC_PROG_CC
LT_INIT([dlopen])

AC_DEFUN([VERSION_TO_NUMBER],
[`$1 | sed -e 's/libxml //' | awk 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3;}'`])

##########################################################################
# debug compilation support
##########################################################################
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [enable debug data generation [default=no]])],
    [debugit="$enableval"],
    [debugit=no])
AC_MSG_RESULT(Enable debug support: [$debugit])

if test x"$debugit" = x"yes"; then
    AC_DEFINE([WITH_DEBUG],[1],[Debug Mode])
    CFLAGS="-g -Wall -Werror -Wno-uninitialized -O0 -pedantic"
fi
AM_CONDITIONAL([WITH_DEBUG], [test x$debug = xtrue])

##########################################################################
# GLib support
##########################################################################
PKG_CHECK_MODULES([GLIB], [glib-3.0], [AC_DEFINE([HAVE_GLIB], [], [Use GLIB])], [WITH_GLIB=no])

##########################################################################
# OpenSSL support
##########################################################################
AC_ARG_WITH(ssl,
[  --with-ssl          add OpenSSL support [[default=no]] ])

if test "$with_ssl" = ""
then
    with_ssl="no"
fi

OPENSSL_LIBS=""
WITH_OPENSSL=0

AC_ARG_WITH(openssl-inc,
    AS_HELP_STRING([--with-openssl-inc=DIR], [where the OpenSSL header files are located]),
    OPENSSL_CPPFLAGS="-I$withval"
)

AC_ARG_WITH(openssl-lib,
    AS_HELP_STRING([--with-openssl-lib=DIR], [where the OpenSSL library files are located]),
    OPENSSL_LDFLAGS="-L$withval"
)

SAVE_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $OPENSSL_CPPFLAGS"

SAVE_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"

if test "$with_ssl" = "no" ; then
    echo Disabling OpenSSL support
else
    echo Enabling OpenSSL support

    AC_CHECK_HEADER(openssl/ssl.h,
        AC_CHECK_LIB(pthread, pthread_join,[
        OPENSSL_LIBS="-lssl"
        AC_DEFINE([HAVE_OPENSSL], [], [Enable OpenSSL])
        WITH_OPENSSL="1"]))
fi

AC_SUBST(WITH_OPENSSL)
AC_SUBST(OPENSSL_CPPFLAGS)
AC_SUBST(OPENSSL_LDFLAGS)
AC_SUBST(OPENSSL_LIBS)

LDFLAGS="$SAVE_LDFLAGS"
CPPFLAGS="$SAVE_CPPFLAGS"

##########################################################################
# threads support
##########################################################################
AC_ARG_WITH(threads,
[  --with-threads          add multithread support [[default=no]] ])

if test "$with_threads" = ""
then
    with_threads="no"
fi

THREAD_LIBS=""
WITH_THREADS=0

if test "$with_threads" = "no" ; then
    echo Disabling multithreaded support
else
    echo Enabling multithreaded support

    if test "$with_threads" = "pthread" || test "$with_threads" = "" || test "$with_threads" = "yes" ; then
        AC_CHECK_HEADER(pthread.h,
            AC_CHECK_LIB(pthread, pthread_join,[
            THREAD_LIBS="-lpthread"
            AC_DEFINE([HAVE_LIBPTHREAD], [], [Define if pthread library is there (-lpthread)])
            AC_DEFINE([HAVE_PTHREAD_H], [], [Define if <pthread.h> is there])
            AC_DEFINE([HAVE_THREADS], [], [Enable threads])
            WITH_THREADS="1"]))
    fi
fi
AC_SUBST(THREAD_LIBS)
AC_SUBST(WITH_THREADS)

##########################################################################
# libxslt support
##########################################################################
LIBXSLT_CONFIG_PREFIX=""
LIBXSLT_LIBS=""
LIBXSLT_CFLAGS=""

AC_ARG_WITH(libxslt-prefix,
        [  --with-libxslt-prefix=[PFX]       Specify location of libxslt config],
    LIBXSLT_CONFIG_PREFIX=$withval
)

AC_ARG_WITH(libxslt-include-prefix,
        [  --with-libxslt-include-prefix=[PFX]   Specify location of libxslt headers],
        LIBXSLT_CFLAGS="-I$withval"
)

AC_ARG_WITH(libxslt-libs-prefix,
        [  --with-libxslt-libs-prefix=[PFX]  Specify location of libxslt libs],
        LIBXSLT_LIBS="-L$withval"
)

AC_SUBST(LIBXSLT_REQUIRED_VERSION)
AC_MSG_CHECKING(for libxslt libraries >= $LIBXSLT_REQUIRED_VERSION)
if test "x$LIBXSLT_CONFIG_PREFIX" != "x"
then
    XSLT_CONFIG=${LIBXSLT_CONFIG_PREFIX}/bin/xslt-config
else
    XSLT_CONFIG=xslt-config
fi

if ${XSLT_CONFIG} --libs print > /dev/null 2>&1
then
    XSLTVERS=`$XSLT_CONFIG --version`
    if test VERSION_TO_NUMBER(echo $XSLTVERS) -ge VERSION_TO_NUMBER(echo $LIBXSLT_REQUIRED_VERSION)
    then
        AC_MSG_RESULT($XSLTVERS found)
    else
        AC_MSG_ERROR(Version $XSLTVERS found. You need at least libxslt $LIBXSLT_REQUIRED_VERSION for this version)
    fi
    LIBXSLT_LIBS="$LIBXSLT_LIBS `$XSLT_CONFIG --libs`"
    if test "x$LIBXSLT_SRC" = "x"; then
        LIBXSLT_CFLAGS="$LIBXSLT_CFLAGS `$XSLT_CONFIG --cflags`"
        CPPFLAGS="`$XSLT_CONFIG --cflags` $CPPFLAGS"
    fi
else
    AC_MSG_ERROR([Could not find libxslt anywhere, check ftp://xmlsoft.org/.])
fi

AC_ARG_ENABLE([exslt], AS_HELP_STRING([[--disable-exslt]],[Disable exslt extensions for libxslt. Default enabled ]),
    [ EXSLT="no" ], [ EXSLT="yes" ])

if test "$EXSLT" = "yes"; then
    AC_CHECK_HEADER(libexslt/exslt.h,
        AC_CHECK_LIB(exslt, exsltLibraryVersion,[
            LIBXSLT_LIBS="$LIBXSLT_LIBS -lexslt"
            AC_DEFINE([HAVE_LIBEXSLT], [], [Define if exslt library is there (-lexslt)])
            AC_DEFINE([HAVE_EXSLT_H], [], [Define if <libexslt/exslt.h> is there])
            WITH_EXSLT="1"]))
fi

AC_LINK_IFELSE([AC_LANG_PROGRAM(
[[
#include <libxslt/xsltInternals.h>
]],
[[
    xsltTransformContext ctxt;
    ctxt.maxTemplateDepth = 1000;
]])],
[LIBXSLT_GLOBAL_MAX_DEPTH=no],
[LIBXSLT_GLOBAL_MAX_DEPTH=yes])
if test "$LIBXSLT_GLOBAL_MAX_DEPTH" = "yes"; then
    AC_DEFINE([USE_LIBEXSLT_GLOBAL_MAX_DEPTH], [], [Define if LibXSLT using global max depth])
    AC_MSG_RESULT(LibXSLT global max depth: yes)
else
    AC_MSG_RESULT(LibXSLT global max depth: no)
fi

AC_SUBST(XSLT_CONFIG)
AC_SUBST(LIBXSLT_LIBS)
AC_SUBST(LIBXSLT_CFLAGS)

##########################################################################
# END
##########################################################################

AC_SUBST(CFLAGS)

AC_OUTPUT(Makefile libxsltp/Makefile xslt_processor/Makefile)

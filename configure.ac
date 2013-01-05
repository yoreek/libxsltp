AC_INIT([XSLT processing library], [1.0], [yoreek@yahoo.com],
        [libxsltp], [http://lalala.com])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])

AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
#AC_PROG_LIBTOOL
LT_INIT([dlopen])
#AC_CONFIG_FILES([Makefile xslt/Makefile])

##########################################################################
# debug compilation support
##########################################################################

AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [enable debug data generation (def=no)])],
    [debugit="$enableval"],
    [debugit=no])
AC_MSG_RESULT([$debugit])

if test x"$debugit" = x"yes"; then
    AC_DEFINE([WITH_DEBUG],[1],[Debug Mode])
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -Werror -Wno-uninitialized -O0"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O3"
fi
AM_CONDITIONAL([WITH_DEBUG], [test x$debug = xtrue])

##########################################################################

AC_SUBST([AM_CXXFLAGS])

#FCGI_COMMON_CHECKS

AC_OUTPUT(Makefile libxsltp/Makefile xslt_processor/Makefile)

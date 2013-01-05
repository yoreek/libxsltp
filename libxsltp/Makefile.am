#INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/sys -I/usr/include/libxml2
INCLUDES = -I$(top_srcdir) -I/usr/include/libxml2

AM_LDFLAGS = -lxslt -lexslt -lxml2 -lpthread -lssl -L/usr/local/lib

#pkglib_LIBRARIES = libvxpxslt.a

#LDADD =  libvxpxslt.a $(top_srcdir)/sys/libvxpsys.a

#libvxpxslt_a_SOURCES = vxp_xslt.c vxp_xslt_document.c vxp_xslt_document_cache.c \
#           vxp_xslt_stylesheet.c vxp_xslt_stylesheet_cache.c \
#           vxp_xslt_keys.c vxp_xslt_keys_cache.c

lib_LTLIBRARIES = libxsltp-1.0.la
libxsltp_1_0_la_SOURCES =                   \
            xsltp.c                         \
            xsltp_result.c                  \
            xsltp_document_parser.c         \
            xsltp_document_parser_cache.c   \
            xsltp_stylesheet_parser.c       \
            xsltp_stylesheet_parser_cache.c \
            xsltp_keys_cache.c              \
            xsltp_core.c                    \
            xsltp_list.c                    \
            xsltp_string.c                  \
            xsltp_pthread_thread.c
#libvxpxslt_1_0_la_LIBADD = $(top_srcdir)/sys/libvxpsys.la
#libvxpxslt_la_LDFLAGS=-shared

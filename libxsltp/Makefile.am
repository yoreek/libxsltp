INCLUDES = -I$(top_srcdir) -I/usr/include/libxml2

AM_LDFLAGS = -lxslt -lexslt -lxml2 -lpthread -lssl -L/usr/local/lib

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

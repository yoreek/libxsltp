AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = $(LIBXSLT_CFLAGS) $(OPENSSL_CFLAGS)
AM_LDFLAGS = $(LIBXSLT_LIBS) $(THREAD_LIBS) $(OPENSSL_LIBS)

lib_LTLIBRARIES = libxsltp.la
libxsltp_la_SOURCES =                       \
            xsltp.c                         \
            xsltp_result.c                  \
            xsltp_document_parser.c         \
            xsltp_document_parser_cache.c   \
            xsltp_stylesheet_parser.c       \
            xsltp_stylesheet_parser_cache.c \
            xsltp_keys_cache.c              \
            xsltp_profiler.c                \
            xsltp_core.c                    \
            xsltp_list.c                    \
            xsltp_log.c                     \
            xsltp_string.c                  \
            xsltp_pthread_thread.c

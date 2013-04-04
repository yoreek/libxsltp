AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -fPIC $(LIBXSLT_CFLAGS) $(OPENSSL_CFLAGS) $(LIBICU_CFLAGS) $(GLIB_CFLAGS)
AM_LDFLAGS = $(LIBXSLT_LIBS) $(THREAD_LIBS) $(OPENSSL_LIBS) $(LIBICU_LIBS) $(GLIB_LIBS)

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
            xsltp_extension.c               \
            xsltp_pthread_thread.c

include_HEADERS =                           \
            xsltp.h                         \
            xsltp_config.h                  \
            xsltp_core.h                    \
            xsltp_linux_config.h            \
            xsltp_list.h                    \
            xsltp_log.h                     \
            xsltp_md5.h                     \
            xsltp_string.h                  \
            xsltp_thread.h

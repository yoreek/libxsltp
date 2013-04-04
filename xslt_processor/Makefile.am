AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = $(LIBXSLT_CFLAGS) $(OPENSSL_CFLAGS) $(LIBICU_CFLAGS) $(GLIB_CFLAGS)
AM_LDFLAGS = $(LIBXSLT_LIBS) $(THREAD_LIBS) $(OPENSSL_LIBS) $(LIBICU_LIBS) $(GLIB_LIBS)

bin_PROGRAMS = xslt_processor
xslt_processor = xslt_processor.c

LDADD =  $(top_srcdir)/libxsltp/libxsltp.la

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = $(LIBXSLT_CFLAGS)
AM_LDFLAGS = $(LIBXSLT_CFLAGS)

bin_PROGRAMS = xslt_processor
xslt_processor = xslt_processor.c

LDADD =  $(top_srcdir)/libxsltp/libxsltp-1.0.la

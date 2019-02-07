bin_PROGRAMS    = decoder

decoder_SOURCES = main.c
decoder_CFLAGS	= -I$(top_builddir)/inc
decoder_LDFLAGS	= -L$(top_builddir)/src/ -lpipeline -L$(top_builddir)/ext/libswo/libswo/.libs/ -lswo
include_HEADERS = $(top_builddir)/inc

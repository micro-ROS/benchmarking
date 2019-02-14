bin_PROGRAMS    = decoder

decoder_SOURCES = main.c
decoder_CFLAGS	= -I$(top_builddir)/inc
decoder_LDFLAGS	= -L$(top_builddir)/src/					\
		  -lpipeline 							\
		  -L$(top_builddir)/ext/libcjson/.libs/ -lcjson 			\
		  -L$(top_builddir)/ext/libswo/libswo/.libs/ -lswo 		\
		  -L$(top_builddir)/ext/libini/.libs/ -lini

include_HEADERS = $(top_builddir)/inc

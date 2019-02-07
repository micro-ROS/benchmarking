lib_LTLIBRARIES    = libpipeline.la

libpipeline_la_SOURCES = decoder_swo.c \
		     message.c \
		     pipeline.c \
		     processing.c \
		     uart.c

libpipeline_la_CFLAGS  = $(LIBTOOL_INCFLAGS) -I$(top_srcdir)/inc  \
			 -I$(top_srcdir)/ext/libswo/ 
libpipeline_la_LDFLAGS = $(LIBTOOL_LDFLAGS)

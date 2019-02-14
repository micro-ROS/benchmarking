lib_LTLIBRARIES    = libpipeline.la

libpipeline_la_SOURCES =  \
		        config.c 	\
			config_ini.c 	\
		        decoder_swo.c 	\
			file.c		\
			form_cjson.c	\
		        message.c 	\
		        pipeline.c 	\
			pkt_converter.c	\
		        processing.c 	\
		        uart.c

libpipeline_la_CFLAGS  = $(LIBTOOL_INCFLAGS) -I$(top_srcdir)/inc  	\
			 -I$(top_srcdir)/ext/libswo/ 			\
			 -I$(top_srcdir)/ext/libcjson/			\
			 -I$(top_srcdir)/ext/libini/src

libpipeline_la_LDFLAGS = $(LIBTOOL_LDFLAGS)

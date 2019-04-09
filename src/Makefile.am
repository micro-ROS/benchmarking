lib_LTLIBRARIES    = libpipeline.la

libpipeline_la_SOURCES =  \
			config.c 	\
			config_ini.c 	\
			decoder_swo.c 	\
			file.c		\
			form_cjson.c	\
			itm_to_str.c	\
			itm2mem_info.c	\
			message.c 	\
			pipeline.c 	\
			perf_ex.c 	\
			pkt_converter.c	\
			processing.c 	\
			swd_ctrl.c	\
			uart.c

libpipeline_la_CFLAGS  = $(LIBTOOL_INCFLAGS) -I$(abs_top_builddir)/inc  	\
			 -I$(abs_top_builddir)/ext/libcjson/			\
			 -I$(abs_top_builddir)/ext/libini/src			\
			 -I$(abs_top_builddir)/ext/libmfa/inc/ 			\
			 -I$(abs_top_builddir)/ext/libswo/ 			\
			 -I$(abs_top_builddir)/ext/openocd/src

libpipeline_la_LDFLAGS = $(LIBTOOL_LDFLAGS)


TESTS = tests/perf_ex_01
check_PROGRAMS = tests/perf_ex_01

tests_perf_ex_01_SOURCES = tests/perf_ex_01.c
tests_perf_ex_01_CFLAGS =  $(CFLAGS) $(libpipeline_la_CFLAGS) $(CHECK_CFLAGS)
tests_perf_ex_01_LDADD = libpipeline.la $(EXT_LIBS) $(CHECK_LIBS) -lswo		\
			 -lcjson -lini -lopenocd -ljim -lmemfootprint		\
			 $(LD_FLAGS)

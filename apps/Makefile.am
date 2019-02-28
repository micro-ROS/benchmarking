bin_PROGRAMS    = decoder

decoder_SOURCES = main.c
decoder_CFLAGS	= -I$(top_builddir)/inc
decoder_LDFLAGS	= $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim
		
include_HEADER = $(top_builddir)/inc

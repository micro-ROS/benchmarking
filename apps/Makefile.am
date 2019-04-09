bin_PROGRAMS    = decoder mfa

decoder_SOURCES = main.c
decoder_CFLAGS	= -I$(top_builddir)/inc
decoder_LDFLAGS	= $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim	\
		  -lmemfootprint

mfa_SOURCES = mfa.c
mfa_CFLAGS = -I$(top_builddir)/inc
mfa_LDFLAGS = $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim	\
		  -lmemfootprint
		
include_HEADER = $(top_builddir)/inc

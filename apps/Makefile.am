bin_PROGRAMS    = pea mfa imt2str

pea_SOURCES = pea.c
pea_CFLAGS	= -I$(top_builddir)/inc
pea_LDFLAGS	= $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim	\
		  -lmemfootprint

mfa_SOURCES = mfa.c
mfa_CFLAGS = -I$(top_builddir)/inc
mfa_LDFLAGS = $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim	\
		  -lmemfootprint

imt2str_SOURCES = itm2str.c
imt2str_CFLAGS = -I$(top_builddir)/inc
imt2str_LDFLAGS = $(EXT_LIBS) -L$(top_builddir)/src/	\
		  -lpipeline -lcjson -lini -lswo	\
		  -ldl -lpthread -lopenocd -ljim	\
		  -lmemfootprint
		
include_HEADER = $(top_builddir)/inc

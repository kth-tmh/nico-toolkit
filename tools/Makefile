BIN    = ../bin
LIBDIR = ../lib
LIBS   = -L$(LIBDIR) -lnico -lm
CC     = gcc
OPT	= -O -ffast-math 
CFLAGS = -Wall -I$(LIBDIR)

%: %.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OPT) $< -o $(BIN)/$@ $(LIBS)

all: CStats AddGroup CreateNet Lab2Targ Move SetPlast \
	AddStream CResult LabEdit NormGroup SetPrune \
	AddUnit Display LinkGroup NormStream SetType \
	BackProp EditStream LinkUnit Pipe UnJoin \
	Barkfib Excite MakeCep Protect \
	Comment Import MakeDiff Prune \
	Connect Join Melfib Remove \
	Copy KickNet Metricnct Rename

clean: 
	rm -f $(BIN)/* *.o


######## special development versions of the targets below here ###########

#
# for debugging
#
LIBS_G = -L$(LIBDIR) -lnico_g -lm
%_g: %.c 
	$(CC) -DTRAILING_MEMFREE -g $(OPT) $(CFLAGS) \
	$< $(LDFLAGS) -o $(BIN)/$@ $(LIBS_G)
debug: $(addsuffix _g, $(all)) 


#
# for profiling
#
LIBS_P = -L$(LIBDIR) -lnico_p -lm
%_p: %.c 
	$(CC) -DTRAILING_MEMFREE -pg $(OPT) $(CFLAGS) \
	$< $(LDFLAGS) -o $(BIN)/$@ $(LIBS_P)
profile: $(addsuffix _p, $(all)) 


#
# for purify
#
%_pure: %.c
	purify $(CC) -DTRAILING_MEMFREE -g $(OPT) $(CFLAGS) \
	$< $(LDFLAGS) -o $(BIN)/$@ $(LIBS_G)
purify: $(addsuffix _pure, $(all)) 

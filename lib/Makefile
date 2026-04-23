CC	= gcc
OPT	= -O -ffast-math 
MALLOC_H_CFLAG := $(shell printf '\043include <malloc.h>\n' | $(CC) -x c -fsyntax-only - >/dev/null 2>&1 && echo -DHAVE_MALLOC_H)
CFLAGS	= $(OPT) -Wall $(MALLOC_H_CFLAG)

modules = AudioData.o LabelData.o RTDNN.o Speetures.o \
	Command.o LogBook.o RTSim.o Sphere.o \
	Display.o nMath.o Signal.o StdHead.o \
	Goodies.o ParamData.o Simulation.o System.o

all: libnico.a

libnico.a: $(modules) 
	ar rv libnico.a $(modules)

clean:
	rm -f *.o
	rm -f libnico.a

##
## for debugging
##
modules_g = AudioData_g.o LabelData_g.o RTDNN_g.o Speetures_g.o \
	Command_g.o LogBook_g.o RTSim_g.o Sphere_g.o \
	Display_g.o nMath_g.o Signal_g.o StdHead_g.o \
	Goodies_g.o ParamData_g.o Simulation_g.o System_g.o


%_g.o: %.c 
	$(CC) -g -c -o $@ $(CFLAGS) $<

libnico_g.a: $(modules_g) 
	ar rv libnico_g.a $(modules_g)

debug: libnico_g.a


##
## for profiling
##
modules_p = AudioData_p.o LabelData_p.o RTDNN_p.o Speetures_p.o \
	Command_p.o LogBook_p.o RTSim_p.o Sphere_p.o \
	Display_p.o nMath_p.o Signal_p.o StdHead_p.o \
	Goodies_p.o ParamData_p.o Simulation_p.o System_p.o


%_p.o: %.c 
	$(CC) -pg $(OPT) -c -o $@ $(CFLAGS) $<

libnico_p.a: $(modules_p) 
	ar rv libnico_p.a $(modules_p)

prof: libnico_p.a

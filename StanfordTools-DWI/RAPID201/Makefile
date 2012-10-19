include ../Makefile.definitions

CC = g++ -O5

CFLAGS		= -I. -I.. $(SYS_CXXFLAGS)

LDFLAGS		= -L. -L..

.SUFFIXES: .C

SRCS		= RAPID.C build.C collide.C overlap.C 

OBJECTS		= RAPID.o build.o collide.o overlap.o 

CLEAN		= sample_client $(OBJECTS) libRAPID.a *.o *~

default: library

install: libRAPID.a
library: libRAPID.a

libRAPID.a: $(OBJECTS)
	/bin/rm -f libRAPID.a
	${AR} libRAPID.a $(OBJECTS)

.C.o:
	$(CC) ${CFLAGS} -c $<

.c.o:
	$(CC) ${CFLAGS} -c $<

test:
	$(CC) $(SYS_LDFLAGS) -o sample_client sample_client.C -lm -lz -lrpcrt4 -loleaut32 -lole32 -luuid -lwinspool -lwinmm -lshell32 -lcomctl32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 -lcomdlg32  -lRAPID

clean:
	touch clean.a
	/bin/rm -f $(CLEAN)

# Makefile for contrack_gen
#
include ../Makefile.definitions
NAME = contrack_gen
EXE = ${NAME}
DEFINES   =
LIBS      = -lm ${UTIL_LIB} -lRAPID -lnifti -lz
CFLAGS    += -O5 -I.. -I../util $(SYS_CXXFLAGS)
LDFLAGS   = -L../util -L../nifti $(RAPID_LIB_PATH) $(ZLIB_LIB_PATH) $(SYS_LDFLAGS) $(LDFLAGS_ALL)
SRCS= seq_track_gen.cpp tracker.cpp main.cpp
OBJS = $(SRCS:%.cpp=%.o)
LIBRARIES = ../util/libutil.a

default: $(EXE)

$(EXE): $(OBJS) $(LIBRARIES)
	$(CPP) -o $(EXE) $(OBJS) $(LDFLAGS) $(LIBS)

.cpp.o: $*.cpp Makefile
	$(CPP) $(CFLAGS) -c $*.cpp -o $*.o

depend:
	makedepend -f- -- $(CFLAGS) -- $(SRCS) > Makefile.dependencies

clean:
	rm -rf $(OBJS) $(EXE) $(EXE).exe

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif




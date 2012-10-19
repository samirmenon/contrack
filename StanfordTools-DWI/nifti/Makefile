# Makefile for nifti (library for handling nifti IO)
#
#
include ../Makefile.definitions

OUTPUT_DIR = .
CFLAGS = -O5 -DHAVE_ZLIB -I. $(ZLIB_INCLUDE_PATH) $(SYS_CXXFLAGS) $(CFLAGS_ALL)
SRCS = nifti1_io.c znzlib.c fslio.c extended_fslio.c
OBJS = $(SRCS:%.c=%.o) 
TARGET = $(OUTPUT_DIR)/libnifti.a

default: $(TARGET)

$(TARGET) : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

.c.o: $*.c Makefile
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)

depend:
	makedepend -f- -- $(CFLAGS) -- $(SRCS) > Makefile.dependencies

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif


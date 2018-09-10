TARGETS= wavtomp3 

CC_CPP = g++

CFLAGS = -Wall -g  -pthread -std=c++11 -D_FILE_OFFSET_BITS

INCL = -Iinclude/ 

LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
	LIBPATH = -L./lib/linux64
else
	LIBPATH = -L./lib/linux
endif


LINKLIB = -lmp3lame

all: clean $(TARGETS)

$(TARGETS):
	$(CC_CPP) $(CFLAGS) $(INCL) source/config.cpp source/wavtomp3.cpp -o $@ $(LIBPATH) $(LINKLIB)

clean:
	rm -f $(TARGETS)

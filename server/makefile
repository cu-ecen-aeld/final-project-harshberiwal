INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc -Wno-stringop-overflow
CROSS_COMPILE=

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

all: server.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) $(CFLAGS) server.c -o server -lm -lpthread


clean:
	-rm -f *.o *.d
	-rm -f server

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
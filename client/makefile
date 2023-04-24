CROSS_COMPILE=
CC=gcc -Wno-stringop-overflow
SRC=client.c
LIBS=-pthread
CFLAGS=
LINKFLAGS=-lm -lrt

all:
	$(CROSS_COMPILE)$(CC) $(SRC) $(LIBS) $(CFLAGS) $(LINKFLAGS) -o client

clean:
	rm client	
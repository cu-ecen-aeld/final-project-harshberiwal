CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
COMPFLAG = -Wall -Werror
LIBS=-lpigpio			
.PHONY: all
all:
	$(CC) $(COMPFLAG) $(LIBS) -c server.c -o server.o
	$(CC) $(COMPFLAG) $(LIBS) -o server server.o	
	
.PHONY: clean
clean:
	rm -f *.o server
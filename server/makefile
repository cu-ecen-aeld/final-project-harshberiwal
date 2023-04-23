CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
COMPFLAG = -Wall -Werror			
.PHONY: all
all:
	$(CC) $(COMPFLAG) -c server.c -o server.o
	$(CC) $(COMPFLAG) -o server server.o	
	
.PHONY: clean
clean:
	rm -f *.o server	
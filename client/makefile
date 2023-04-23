CC = $(CROSS_COMPILE)gcc
COMPFLAG = -Wall -Werror			
.PHONY: all
all: client
writer :
	$(CC) $(COMPFLAG) -c client.c -o client.o
	$(CC) $(COMPFLAG) -o client client.o	
	
.PHONY: clean
clean:
	rm -f *.o client	
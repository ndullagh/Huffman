CC = gcc

CFLAGS = -ansi -Wall -Werror -pedantic -g

IN = test



hencode: hencode.c htable.c
	$(CC) $(CFLAGS) -o hencode hencode.c htable.c

all: hencode hdecode
	
test: hencode
	./hencode $(IN) > h.out
clean:
	rm hencode.o
	
hdecode: hdecode.c htable.c
	$(CC) $(CFLAGS) -o hdecode hdecode.c htable.c


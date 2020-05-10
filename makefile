CC=gcc
CFLAGS=-Wall -Werror
vmsim: vmsim.c
		$(CC) -o vmsim vmsim.c $(CFLAGS)
clean: 
		$(RM) vmsim

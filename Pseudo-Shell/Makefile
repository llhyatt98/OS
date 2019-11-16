CC = gcc
target = pseudo-shell
sources = $(wildcard *.c)
objects = main.o command.o

$(target) : $(objects)
	$(CC) $(objects) -o pseudo-shell

main.o : main.c
	$(CC) -g -c main.c

command.o : command.c
	$(CC) -g -c command.c
	
clean: 
	rm *.o $(target)

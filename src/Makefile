all: server.o
	gcc server.o main.c -o main -lpthread

server.o: server.h
	gcc -c server.c

clean:
	rm -rf *.o
	rm -rf main

iterative:
	./main 1

fork:
	./main 2

thread:
	./main 3

concurrent:
	./main 4

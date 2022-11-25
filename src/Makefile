all: server.o
	gcc server.o main.c -o main -lpthread

server.o: server.h
	gcc -c server.c

clean:
	rm -rf *.o
	rm -rf main

iterative:
	./main 1 2000

fork:
	./main 2 2077

thread:
	./main 3 3033

concurrent:
	./main 4 4044

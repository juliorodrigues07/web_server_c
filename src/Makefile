all: server.o
	gcc server.o main.c -o main -lpthread

server.o: server.h
	gcc -c server.c -lpthread

clean:
	rm -rf *.o
	rm -rf main

run:
	./main 3
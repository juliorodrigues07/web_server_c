all: server.o
	gcc server.o main.c -o main

server.o: server.h
	gcc -c server.c

clean:
	rm -rf *.o
	rm -rf main

run:
	./main 1
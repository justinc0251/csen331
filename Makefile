all: server client

server: frame.o server.c
	gcc frame.o server.c -o server

client: frame.o client.c
	gcc frame.o client.c -o client

frame.o: frame.c frame.h
	gcc -c frame.c -o frame.o

clean:
	rm -f *.o server client

run-server: server
	./server

run-client: client
	./client
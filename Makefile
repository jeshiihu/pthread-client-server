CC = gcc

SOURCES = server.c client.c 

all: server client

server: server.c 
	gcc -g -Wall -o server server.c -lpthread

client: client.c
	gcc -g -Wall -o client client.c -lpthread

clean:
	-rm -f *.o server client

run:
	./server ./client
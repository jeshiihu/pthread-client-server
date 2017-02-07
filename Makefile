CC = gcc

SOURCES = server.c client.c rwlockserver.c 

all: server client rwserver

server: server.c 
	gcc -g -Wall -o server server.c -lpthread

client: client.c
	gcc -g -Wall -o client client.c -lpthread

rwserver: rwlockserver.c
	gcc -g -Wall -o rwserver rwlockserver.c -lpthread

clean:
	-rm -f *.o server client rwserver

run:
	./server ./client ./rwserver
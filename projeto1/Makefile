CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c movie.h
	$(CC) $(CFLAGS) server.c -o server

client: client.c movie.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client 
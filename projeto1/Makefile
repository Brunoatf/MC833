CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c json_database.c cJSON.c movie.h
	$(CC) $(CFLAGS) server.c json_database.c cJSON.c -o server -lpthread

client: client.c movie.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client 
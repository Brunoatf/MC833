CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client movies.json

server: server.c json_database.c cJSON.c movie.h
	$(CC) $(CFLAGS) server.c json_database.c cJSON.c -o server -pthread

client: client.c movie.h
	$(CC) $(CFLAGS) client.c -o client

movies.json:
	@echo "[]" > movies.json

clean:
	rm -f server client movies.json

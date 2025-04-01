#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3490
#define MAXDATASIZE 100

#define MAX_TITLE_LEN 100
#define MAX_DIRECTOR_LEN 100
#define MAX_GENRES 3
#define MAX_GENRE_LEN 20

struct movie {
    int id;
    char title[MAX_TITLE_LEN];
    char genres[MAX_GENRES][MAX_GENRE_LEN];
    int genre_count;
    char director[MAX_DIRECTOR_LEN];
    int year;
};

struct add_genre_payload {
    int id;
    char *genre;
};

struct request_movie_addition {
    int operation;
    struct movie m;
};

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct sockaddr_in server_addr;

    if (argc != 2) {
        fprintf(stderr, "uso: %s <IP do servidor>\n", argv[0]);
        exit(1);
    }

    // Cria o socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // Preenche dados do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), '\0', 8);

    // Conecta ao servidor
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    struct movie m = {
        .id = 1,
        .year = 1994,
        .genre_count = 2
    };
    strcpy(m.title, "The Shawshank Redemption");
    strcpy(m.genres[0], "Drama");
    strcpy(m.genres[1], "Crime");
    strcpy(m.director, "Frank Darabont");

    int operation = 1;
    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, &m, sizeof(m), 0);

    // Recebe dados
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);

    close(sockfd);
    return 0;
}

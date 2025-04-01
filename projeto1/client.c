#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "movie.h"

#define MAXDATASIZE 100

// Funções auxiliares para cada operação
static void send_save_movie(int sockfd) {
    struct movie m = {
        .year = 1994,
        .genre_count = 2
    };
    strcpy(m.title, "The Shawshank Redemption");
    strcpy(m.genres[0], "Drama");
    strcpy(m.genres[1], "Crime");
    strcpy(m.director, "Frank Darabont");

    int operation = OP_SAVE_MOVIE;
    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, &m, sizeof(m), 0);
}

static void send_add_genre(int sockfd) {
    int operation = OP_ADD_GENRE;
    struct genre_addition_params gap;
    gap.id = 1;
    strcpy(gap.genre, "Drama");

    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, &gap, sizeof(gap), 0);
}

static void send_remove_movie(int sockfd) {
    int operation = OP_REMOVE_MOVIE;
    int id = 1;

    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, &id, sizeof(int), 0);
}

static void send_list_titles(int sockfd) {
    int operation = OP_LIST_TITLES;
    send(sockfd, &operation, sizeof(int), 0);
}

static void send_list_all(int sockfd) {
    int operation = OP_LIST_ALL;
    send(sockfd, &operation, sizeof(int), 0);
}

static void send_list_by_genre(int sockfd) {
    int operation = OP_LIST_BY_GENRE;
    char genre[MAX_GENRE_LEN] = "Drama";

    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, genre, sizeof(genre), 0);
}

static void send_list_by_year(int sockfd) {
    int operation = OP_LIST_BY_YEAR;
    int year = 1994;

    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, &year, sizeof(year), 0);
}

static void send_list_by_director(int sockfd) {
    int operation = OP_LIST_BY_DIRECTOR;
    char director[MAX_DIRECTOR_LEN] = "Frank Darabont";

    send(sockfd, &operation, sizeof(int), 0);
    send(sockfd, director, sizeof(director), 0);
}

int main(int argc, char *argv[]) {
    /*
     * Cliente para o servidor de filmes.
     * 
     * Uso: ./client <IP do servidor>
     * 
     * Exemplo: ./client 127.0.0.1
     */

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

    // Exemplo de uso: remove um filme
    send_remove_movie(sockfd);

    // Recebe resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);

    close(sockfd);
    return 0;
}

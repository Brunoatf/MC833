
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "movie.h"

#define MAXDATASIZE 4096

void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void read_string(const char *prompt, char *dest, int max_len) {
    printf("%s", prompt);
    fgets(dest, max_len, stdin);
    dest[strcspn(dest, "\n")] = '\0'; 
}

void read_int(const char *prompt, int *value) {
    printf("%s", prompt);
    scanf("%d", value);
    flush_input();
}

void send_request(int sockfd, int operation, void *data, size_t data_size) {
    send(sockfd, &operation, sizeof(int), 0);
    if (data && data_size > 0)
        send(sockfd, data, data_size, 0);
    
    char buf[MAXDATASIZE];
    int numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
    if (numbytes > 0) {
        buf[numbytes] = '\0';
        printf("\nResposta do servidor:\n%s\n\n", buf);
    } else {
        printf("Erro ao receber resposta do servidor.\n");
    }
}

void menu_loop(int sockfd) {
    while (1) {
        printf("Consulta para Streaming de Filmes:\n");
        printf("1. Adicionar filme\n");
        printf("2. Adicionar gênero a filme\n");
        printf("3. Remover filme\n");
        printf("4. Listar todos os títulos e ids\n");
        printf("5. Listar todos os filmes\n");
        printf("6. Buscar filme por ID\n");
        printf("7. Buscar filmes por gênero\n");
        printf("0. Sair\n");
        printf("Opção: ");

        int op;
        scanf("%d", &op);
        flush_input();

        switch (op) {
            case 1: {
                struct movie m;
                m.id = 0; // o servidor define
                read_string("Título: ", m.title, MAX_TITLE_LEN);
                read_string("Diretor: ", m.director, MAX_DIRECTOR_LEN);
                read_int("Ano: ", &m.year);
                read_int("Quantidade de gêneros (máx 3): ", &m.genre_count);

                if (m.genre_count > MAX_GENRES) m.genre_count = MAX_GENRES;
                for (int i = 0; i < m.genre_count; i++) {
                    char prompt[50];
                    snprintf(prompt, sizeof(prompt), "Gênero %d: ", i + 1);
                    read_string(prompt, m.genres[i], MAX_GENRE_LEN);
                }

                send_request(sockfd, OP_SAVE_MOVIE, &m, sizeof(m));
                break;
            }
            case 2: {
                struct genre_addition_params gap;
                read_int("ID do filme: ", &gap.id);
                read_string("Novo gênero: ", gap.genre, MAX_GENRE_LEN);
                send_request(sockfd, OP_ADD_GENRE, &gap, sizeof(gap));
                break;
            }
            case 3: {
                int id;
                read_int("ID do filme a remover: ", &id);
                send_request(sockfd, OP_REMOVE_MOVIE, &id, sizeof(id));
                break;
            }
            case 4:
                send_request(sockfd, OP_LIST_ALL_MOVIES_TITLES_AND_IDS, NULL, 0);
                break;
            case 5:
                send_request(sockfd, OP_LIST_ALL_MOVIES_INFO, NULL, 0);
                break;
            case 6: {
                int id;
                read_int("ID do filme: ", &id);
                send_request(sockfd, OP_LIST_MOVIE_BY_ID, &id, sizeof(id));
                break;
            }
            case 7: {
                char genre[MAX_GENRE_LEN];
                read_string("Gênero: ", genre, MAX_GENRE_LEN);
                send_request(sockfd, OP_LIST_MOVIES_BY_GENRE, genre, sizeof(genre));
                break;
            }
            case 0:
                printf("Encerrando...\n");
                return;
            default:
                printf("Opção inválida!\n");
                break;
        }
        printf("----------------------------------------\n");
        return;
    }
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

    // Faz o display de uma interface de filmes
    menu_loop(sockfd);

    // Recebe resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client:- received :\n%s\n", buf);

    close(sockfd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "movie.h"

#define MAXDATASIZE 4096

void send_request(int sockfd, int operation, void *data, size_t data_size) {
    /* 
    * Envia uma operação e seus dados ao servidor via socket.
    * Depois, recebe e exibe a resposta do servidor.
    */
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

void display_interface(int sockfd) {
    /* 
    * Exibe a interface de opções do cliente, 
    * coleta dados do usuário e chama a função de envio adequada.
    */
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
    getchar(); 

    switch (op) {
        case 1: {
            struct movie m;
            m.id = 0;

            printf("Título: ");
            fgets(m.title, MAX_TITLE_LEN, stdin);
            m.title[strcspn(m.title, "\n")] = '\0';

            printf("Diretor: ");
            fgets(m.director, MAX_DIRECTOR_LEN, stdin);
            m.director[strcspn(m.director, "\n")] = '\0';

            printf("Ano: ");
            scanf("%d", &m.year);

            printf("Quantidade de gêneros (mín 1): ");
            scanf("%d", &m.genre_count);
            getchar(); 

            if (m.genre_count > MAX_GENRES) m.genre_count = MAX_GENRES;

            for (int i = 0; i < m.genre_count; i++) {
                printf("Gênero %d: ", i + 1);
                fgets(m.genres[i], MAX_GENRE_LEN, stdin);
                m.genres[i][strcspn(m.genres[i], "\n")] = '\0';
            }

            send_request(sockfd, OP_SAVE_MOVIE, &m, sizeof(m));
            break;
        }

        case 2: {
            struct genre_addition_params gap;

            printf("ID do filme: ");
            scanf("%d", &gap.id);
            getchar();

            printf("Novo gênero: ");
            fgets(gap.genre, MAX_GENRE_LEN, stdin);
            gap.genre[strcspn(gap.genre, "\n")] = '\0';

            send_request(sockfd, OP_ADD_GENRE, &gap, sizeof(gap));
            break;
        }

        case 3: {
            int id;
            printf("ID do filme a remover: ");
            scanf("%d", &id);
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
            printf("ID do filme: ");
            scanf("%d", &id);
            send_request(sockfd, OP_LIST_MOVIE_BY_ID, &id, sizeof(id));
            break;
        }

        case 7: {
            char genre[MAX_GENRE_LEN];
            printf("Gênero: ");
            fgets(genre, MAX_GENRE_LEN, stdin);
            genre[strcspn(genre, "\n")] = '\0';
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
    display_interface(sockfd);

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

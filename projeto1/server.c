#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "movie.h"

static void handle_save_movie(int socket_fd) {
    struct movie m;
    if (recv(socket_fd, &m, sizeof(m), 0) <= 0) {
        perror("recv movie");
        return;
    }

    printf("Adicionar filme: %s (%d), dirigido por %s\n", m.title, m.year, m.director);
    printf("Gêneros (%d):\n", m.genre_count);
    for (int i = 0; i < m.genre_count; i++) {
        printf("- %s\n", m.genres[i]);
    }
}

static void handle_add_genre(int socket_fd) {
    struct genre_addition_params gap;
    if (recv(socket_fd, &gap, sizeof(gap), 0) <= 0) {
        perror("recv gender addition");
        return;
    }

    printf("Adicionar gênero: %s ao filme com id %d\n", gap.genre, gap.id);
}

static void handle_remove_movie(int socket_fd) {
    int id;
    if (recv(socket_fd, &id, sizeof(int), 0) <= 0) {
        perror("recv id for removing movie");
        return;
    }

    printf("Remover filme com o id %d\n", id);
}

static void handle_list_titles(int socket_fd) {
    printf("Listar títulos e ids de todos os filmes\n");
}

static void handle_list_all(int socket_fd) {
    printf("Listar todos os dados de todos os filmes\n");
}

static void handle_list_by_genre(int socket_fd) {
    char genre[MAX_GENRE_LEN];
    if (recv(socket_fd, genre, sizeof(genre), 0) <= 0) {
        perror("recv genre");
        return;
    }

    printf("Listar todos os filmes de gênero %s\n", genre);
}

static void handle_list_by_year(int socket_fd) {
    int year;
    if (recv(socket_fd, &year, sizeof(year), 0) <= 0) {
        perror("recv year");
        return;
    }

    printf("Listar todos os filmes do ano %d\n", year);
}

static void handle_list_by_director(int socket_fd) {
    char director[MAX_DIRECTOR_LEN];
    if (recv(socket_fd, director, sizeof(director), 0) <= 0) {
        perror("recv director");
        return;
    }

    printf("Listar todos os filmes de um determinado diretor %s\n", director);
}

static void handle_invalid_operation(int socket_fd, int operation) {
    printf("Operação inválida: %d\n", operation);
    char error_msg[] = "Operação inválida";
    send(socket_fd, error_msg, strlen(error_msg), 0);
}

void process_message(int socket_fd) {
    /*
     * Processa as mensagens recebidas do cliente através do socket.
     * 
     * Parâmetros:
     *   socket_fd: Descritor do socket para comunicação com o cliente
     * 
     * A função recebe um inteiro indicando a operação desejada e processa
     * diferentes tipos de requisições relacionadas a filmes.
     */

    int operation;
    if (recv(socket_fd, &operation, sizeof(int), 0) <= 0) {
        perror("recv operation");
        return;
    }

    switch (operation) {
        case OP_SAVE_MOVIE:
            handle_save_movie(socket_fd);
            break;
        case OP_ADD_GENRE:
            handle_add_genre(socket_fd);
            break;
        case OP_REMOVE_MOVIE:
            handle_remove_movie(socket_fd);
            break;
        case OP_LIST_TITLES:
            handle_list_titles(socket_fd);
            break;
        case OP_LIST_ALL:
            handle_list_all(socket_fd);
            break;
        case OP_LIST_BY_GENRE:
            handle_list_by_genre(socket_fd);
            break;
        case OP_LIST_BY_YEAR:
            handle_list_by_year(socket_fd);
            break;
        case OP_LIST_BY_DIRECTOR:
            handle_list_by_director(socket_fd);
            break;
        default:
            handle_invalid_operation(socket_fd, operation);
            break;
    }
}

int main() {
    /*
     * Função principal do servidor que:
     * 1. Inicializa o socket do servidor
     * 2. Configura o endereço e porta para escuta
     * 3. Inicia o loop principal de aceitação de conexões
     * 4. Para cada nova conexão:
     *    - Cria um processo filho para atendê-la
     *    - Processa as mensagens do cliente
     *    - Fecha a conexão após o processamento
     * 
     * Retorna:
     *   0 em caso de sucesso
     *   1 em caso de erro na inicialização do socket
     */

    int sockfd, new_fd; // sockfd é para o socket que faz o servidor escutar e new_fd é o usado para conexões TCP novas
    struct sockaddr_in servaddr, cliaddr;
    socklen_t sin_size;
    char client_ip[INET_ADDRSTRLEN];

    // Criamos o socket que escutará as requests no server:
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation");
        exit(1);
    }

    // Configuramos o socket para se ligar à porta desejada:
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(servaddr.sin_zero), '\0', 8);
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Avisamos ao socket que ele deve começar a ouvir novas mensagens:
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1) {
        sin_size = sizeof(cliaddr);
        new_fd = accept(sockfd, (struct sockaddr *)&cliaddr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, sizeof(client_ip));
        printf("server: got connection from %s\n", client_ip);

        // fork cria um processo filho com return == 0 para ser usado para atender ao cliente recebido:
        if (!fork()) {
            close(sockfd);
            send(new_fd, "Hello, world!", 13, 0);
            process_message(new_fd);
            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

    return 0;
}

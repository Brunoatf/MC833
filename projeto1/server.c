#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "movie.h"
#include "json_database.h"
#include <pthread.h>

//Mutex para resolver o problema de concorrencia
pthread_mutex_t json_mutex = PTHREAD_MUTEX_INITIALIZER;


static void handle_save_movie(int socket_fd) {
    /* 
    * Recebe um filme do cliente, salva no banco de dados e 
    * envia uma mensagem de confirmação com o ID ou erro.
    */
    struct movie m;
    printf("operation: handle_save_movie\n");

    if (recv(socket_fd, &m, sizeof(m), 0) <= 0) {
        perror("recv movie");
        return;
    }

    pthread_mutex_lock(&json_mutex);
    int id = add_movie(&m);
    pthread_mutex_unlock(&json_mutex);

    char msg[100];
    if (id >= 0) snprintf(msg, sizeof(msg), "Filme '%d' adicionado com sucesso !", id);
    else snprintf(msg, sizeof(msg),"Erro ao salvar filme");

    send(socket_fd, msg, strlen(msg), 0);

}

static void handle_add_genre(int socket_fd) {
    /* 
    * Recebe um ID de filme e um novo gênero do cliente,
    * tenta adicionar o gênero ao filme e envia o resultado.
    */
    struct genre_addition_params gap;
    printf("operation: handle_add_genre");

    if (recv(socket_fd, &gap, sizeof(gap), 0) <= 0) {
        perror("recv genre addition");
        return;
    }

    pthread_mutex_lock(&json_mutex);
    int result = add_genre(gap.id, gap.genre);
    pthread_mutex_unlock(&json_mutex);

    if (result == 1) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Gênero \"%s\" adicionado ao filme de id %d.", gap.genre, gap.id);
        send(socket_fd, msg, strlen(msg), 0);
    } else if (result == -1) {
        char msg[] = "Filme já possui esse gênero.";
        send(socket_fd, msg, strlen(msg), 0);
    } else if (result == -2) {
        char msg[] = "Limite de gêneros atingido.";
        send(socket_fd, msg, strlen(msg), 0);
    } else {
        char msg[] = "Filme não encontrado.";
        send(socket_fd, msg, strlen(msg), 0);
    }
}


static void handle_remove_movie(int socket_fd) {
    /* 
    * Recebe o ID de um filme e remove o filme do banco.
    * Retorna ao cliente uma mensagem de sucesso ou erro.
    */
    int id;
    printf("operation: handle_remove_movie");

    if (recv(socket_fd, &id, sizeof(int), 0) <= 0) {
        perror("recv id for removing movie");
        return;
    }

    pthread_mutex_lock(&json_mutex);
    int result = remove_movie(id);
    pthread_mutex_unlock(&json_mutex);

    if (result) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Filme com id %d removido com sucesso.", id);
        send(socket_fd, msg, strlen(msg), 0);
    } else {
        char msg[100];
        snprintf(msg, sizeof(msg), "Filme com id %d não encontrado.", id);
        send(socket_fd, msg, strlen(msg), 0);
    }
}


static void handle_list_titles(int socket_fd) {
    /* 
    * Lista todos os filmes cadastrados mostrando apenas
    * seus IDs e títulos. Envia o resultado ao cliente.
    */
    struct movie movies[MAX_MOVIES];
    printf("operation: handle_list_titles");

    int count = list_movies(movies, MAX_MOVIES);

    char buffer[2048] = {0};
    for (int i = 0; i < count; i++) {
        char line[200];
        snprintf(line, sizeof(line), "ID: %d - Título: %s\n", movies[i].id, movies[i].title);
        strcat(buffer, line);
    }

    if (count == 0) {
        strcpy(buffer, "Nenhum filme cadastrado.");
    }

    send(socket_fd, buffer, strlen(buffer), 0);
}


static void handle_list_all(int socket_fd) {
    /* 
    * Lista todas as informações completas de todos os filmes 
    * cadastrados e envia em formato formatado ao cliente.
    */
    struct movie movies[MAX_MOVIES];
    printf("operation: handle_list_all");

    int count = list_movies(movies, MAX_MOVIES);

    char buffer[4096] = {0};
    for (int i = 0; i < count; i++) {
        char line[512];
        snprintf(line, sizeof(line),
            "ID: %d\nTítulo: %s\nDiretor: %s\nAno: %d\nGêneros: ",
            movies[i].id, movies[i].title, movies[i].director, movies[i].year);

        strcat(buffer, line);
        for (int j = 0; j < movies[i].genre_count; j++) {
            strcat(buffer, movies[i].genres[j]);
            if (j < movies[i].genre_count - 1) strcat(buffer, ", ");
        }
        strcat(buffer, "\n\n");
    }

    if (count == 0) {
        strcpy(buffer, "Nenhum filme cadastrado.");
    }

    send(socket_fd, buffer, strlen(buffer), 0);
}


static void handle_list_by_id(int socket_fd) {
    /* 
    * Recebe um ID do cliente, busca o filme correspondente 
    * e envia suas informações completas. Caso não encontre, avisa.
    */
    int id;
    printf("operation: handle_list_by_id");

    if (recv(socket_fd, &id, sizeof(id), 0) <= 0) {
        perror("recv id");
        return;
    }

    struct movie m;

    pthread_mutex_lock(&json_mutex);
    if (get_movie_by_id(id, &m)) {
        char msg[512];
        snprintf(msg, sizeof(msg),
            "ID: %d\nTítulo: %s\nDiretor: %s\nAno: %d\nGêneros: ",
            m.id, m.title, m.director, m.year);

        for (int i = 0; i < m.genre_count; i++) {
            strcat(msg, m.genres[i]);
            if (i < m.genre_count - 1) strcat(msg, ", ");
        }

        send(socket_fd, msg, strlen(msg), 0);
    } else {
        char *msg = "Filme não encontrado.";
        send(socket_fd, msg, strlen(msg), 0);
    }
    pthread_mutex_unlock(&json_mutex);

}


static void handle_list_by_genre(int socket_fd) {
    /* 
    * Recebe um gênero do cliente, filtra os filmes com esse gênero 
    * e envia os resultados encontrados. Caso nenhum, informa.
    */
    char genre[MAX_GENRE_LEN];
    printf("operation: handle_list_by_genre");

    if (recv(socket_fd, genre, sizeof(genre), 0) <= 0) {
        perror("recv genre");
        return;
    }

    struct movie movies[MAX_MOVIES];

    pthread_mutex_lock(&json_mutex);
    int count = list_movies_by_genre(genre, movies, MAX_MOVIES);
    pthread_mutex_unlock(&json_mutex);

    char buffer[4096] = {0};
    for (int i = 0; i < count; i++) {
        char line[512];
        snprintf(line, sizeof(line),
            "ID: %d\nTítulo: %s\nDiretor: %s\nAno: %d\n\n",
            movies[i].id, movies[i].title, movies[i].director, movies[i].year);
        strcat(buffer, line);
    }

    if (count == 0) {
        snprintf(buffer, sizeof(buffer), "Nenhum filme encontrado no gênero \"%s\".", genre);
    }

    send(socket_fd, buffer, strlen(buffer), 0);
}

static void handle_invalid_operation(int socket_fd, int operation) {
    /* 
    * Envia uma mensagem de erro ao cliente indicando que 
    * a operação recebida não é válida.
    */
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
        case OP_LIST_ALL_MOVIES_TITLES_AND_IDS:
            handle_list_titles(socket_fd);
            break;
        case OP_LIST_ALL_MOVIES_INFO:
            handle_list_all(socket_fd);
            break;
        case OP_LIST_MOVIE_BY_ID:
            handle_list_by_id(socket_fd);
            break;
        case OP_LIST_MOVIES_BY_GENRE:
            handle_list_by_genre(socket_fd);
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

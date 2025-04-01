#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 3490
#define BACKLOG 10

#define MAX_TITLE_LEN 100
#define MAX_DIRECTOR_LEN 100
#define MAX_GENRES 3
#define MAX_GENRE_LEN 20

struct movie {
    char title[MAX_TITLE_LEN];
    char genres[MAX_GENRES][MAX_GENRE_LEN];
    int genre_count;
    char director[MAX_DIRECTOR_LEN];
    int year;
};

struct genre_addition_params {
    int id;
    char genre[MAX_GENRE_LEN];
};

void process_message(int socket_fd) {
    printf("process_message()\n");

    // Esperamos para receber um inteiro indicando a operação que o cliente deseja fazer:
    int operation;
    if (recv(socket_fd, &operation, sizeof(int), 0) <= 0) {
        perror("recv operation");
        return;
    }

    switch (operation) {
        case 1: // Salvar filme no banco
            {
                struct movie m;
                if (recv(socket_fd, &m, sizeof(m), 0) <= 0) {
                    perror("recv movie");
                    return;
                }

                // Prints para debug, podem ser removidos depois:
                printf("Adicionar filme: %s (%d), dirigido por %s\n", m.title, m.year, m.director);
                printf("Gêneros (%d):\n", m.genre_count);
                for (int i = 0; i < m.genre_count; i++) {
                    printf("- %s\n", m.genres[i]);
                }
            }
            break;
        case 2: // Adicionar um novo gênero a um filme
            {
                struct genre_addition_params gap;
                if (recv(socket_fd, &gap, sizeof(gap), 0) <= 0) {
                    perror("recv gender addition");
                    return;
                }

                printf("Adicionar gênero: %s ao filme com id %d\n", gap.genre, gap.id);
            }
            break;
        case 3: // Remover um filme pelo identificador
            {
                int id;
                if (recv(socket_fd, &id, sizeof(int), 0) <= 0) {
                    perror("recv id for removing movie");
                    return;
                }

                printf("Remover filme com o id %d\n", id);
            }
            break; 

        case 4:
            // ... existing code ...
            break;

        case 5:
            // ... existing code ...
            break;

        case 6:
            // ... existing code ...
            break;

        case 7:
            // ... existing code ...
            break;

        case 8:
            // ... existing code ...
            break;

        default:
            // ... existing code ...
            break;
    }
}

int main() {
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

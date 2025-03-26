#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 3490
#define BACKLOG 10

int main() {
    int sockfd, new_fd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t sin_size;
    char client_ip[INET_ADDRSTRLEN];

    // Cria socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation");
        exit(1);
    }

    // Preenche struct do servidor
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;  // Aceita qualquer IP local
    memset(&(servaddr.sin_zero), '\0', 8);  // Limpa o resto da struct

    // Associa o socket à porta
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Coloca o socket em modo de escuta
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

        // Converte IP do cliente para string e imprime
        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, sizeof(client_ip));
        printf("server: got connection from %s\n", client_ip);

        if (!fork()) {
            // Processo filho
            close(sockfd);
            send(new_fd, "Hello, world!", 13, 0);
            close(new_fd);
            exit(0);
        }

        close(new_fd);  // Processo pai fecha conexão
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int status;
    char buffer[BUFFER_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip/hostname> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *host = argv[1];
    char *porta = argv[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_family = AF_UNSPEC;     

    if ((status = getaddrinfo(host, porta, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        // socket()
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); [cite: 94]
        if (sockfd < 0) {
            perror("cliente: ERRO socket()");
            continue;
        }

        // connect()
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) { [cite: 190]
            perror("cliente: ERRO connect()");
            close(sockfd);
            continue;
        }

        break; 
    }

    if (p == NULL) {
        fprintf(stderr, "Cliente: Falha ao conectar em qualquer endereço.\n");
        freeaddrinfo(servinfo);
        exit(2);
    }
   
    char ip_str[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), ip_str, sizeof ip_str);
    printf("Cliente: Conectado ao servidor %s (%s)\n", host, ip_str);

    freeaddrinfo(servinfo);

    // recv()
    int numbytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0); [cite: 208]
    if (numbytes < 0) {
        perror("recv");
        close(sockfd);
        exit(1);
    } else if (numbytes == 0) {
        printf("Cliente: Servidor fechou a conexão inesperadamente.\n");
        close(sockfd);
        exit(1);
    }

    buffer[numbytes] = '\0';
    printf("Cliente: Mensagem do Servidor: %s", buffer);

    // send()
    char *msg_resposta = "Ola servidor! Sou o cliente.\n";
    if (send(sockfd, msg_resposta, strlen(msg_resposta), 0) == -1) { [cite: 195]
        perror("send");
    } else {
        printf("Cliente: Mensagem de resposta enviada.\n");
    }

    // close()
    close(sockfd); [cite: 214]
    printf("Cliente: Conexão fechada.\n");

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>          // getaddrinfo
#include <arpa/inet.h>      // inet_ntop

#define BUFFER_SIZE 1024
#define BACKLOG 10

// Função para obter o endereço (IPv4 ou IPv6)
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) { // IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}

int main(int argc, char *argv[]) {
    int sockfd;             // para escuta
    int new_sockfd;         // para conexão
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // conexão do cliente
    socklen_t cli_len;
    char cli_ip_str[INET6_ADDRSTRLEN]; // IP do cliente
    int status;
    char buffer[BUFFER_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <protocolo: v4/v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *protocolo = argv[1]; 
    char *porta = argv[2];

    // Configuração dos hints para getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // usar meu IP (para bind)

    if (strcmp(protocolo, "v4") == 0) {
        hints.ai_family = AF_INET; 
    } else if (strcmp(protocolo, "v6") == 0) {
        hints.ai_family = AF_INET6; 
    } else {
        fprintf(stderr, "Protocolo inválido. Use 'v4' ou 'v6'.\n");
        exit(EXIT_FAILURE);
    }

    if ((status = getaddrinfo(NULL, porta, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // socket() e bind()
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            perror("servidor: ERRO socket()");
            continue;
        }

        // Permite reusar a porta rapidamente (opcional, mas útil)
        // setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            perror("servidor: ERRO bind()");
            close(sockfd);
            continue;
        }

        break; 
    }    

    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) < 0) { 
        perror("servidor: ERRO listen()");
        exit(1);
    }

    printf("Servidor aguardando conexões na porta %s (Protocolo: %s)...\n", porta, protocolo);

    // Loop principal de conexão
    while (1) {
        cli_len = sizeof their_addr;
        
        // accept()
        new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &cli_len);
        if (new_sockfd < 0) {
            perror("servidor: ERRO accept()");
            continue; 
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
                  cli_ip_str, sizeof cli_ip_str);
        printf("Servidor: Conexão recebida de %s\n", cli_ip_str);

        // Envia mensagem de boas-vindas
        char *msg_bem_vindo = "Conexao estabelecida com sucesso!\n";
        if (send(new_sockfd, msg_bem_vindo, strlen(msg_bem_vindo), 0) == -1) { 
            perror("send");
        }

        // Recebe resposta do cliente
        int numbytes = recv(new_sockfd, buffer, BUFFER_SIZE - 1, 0); 
        if (numbytes < 0) {
            perror("recv");
        } else if (numbytes == 0) {
            printf("Servidor: Cliente (IP: %s) desconectou.\n", cli_ip_str);
        } else {
            buffer[numbytes] = '\0';
            printf("Servidor: Mensagem do cliente (IP: %s): %s", cli_ip_str, buffer);
        }

        close(new_sockfd); 
        printf("Servidor: Conexão com %s fechada. Aguardando novos clientes...\n", cli_ip_str);
    }

    close(sockfd);
    return 0;
}

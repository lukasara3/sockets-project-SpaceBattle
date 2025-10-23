#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>          #getaddrinfo
#include <arpa/inet.h>      #inet_ntop

#define BUFFER_SIZE 1024
#define BACKLOG 10

// Ao inves de usar manualmente socketaddr_in e socketaddr_in6, vou usar essa funcao para o endereço
// ponteiro para poder apontar para v4 ou v6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) { //v4, se a familia for do v4, usa v4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); //v6
}

int main(int argc, char *argv[]) {
    int sockfd; //para escuta
    int new_sockfd; //para conexao
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; //conexao do cliente
    socklen_t cli_len;
    char cli_ip_str[INET6_ADDRSTRLEN]; // IP do cliente
    int status;
    int yes = 1;
    char buffer[BUFFER_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <endereço do servidor> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *protocolo = argv[1]; //o numero 4 do enunciado diz que o primeiro argumento eh o protocolo v4 ou v6 e o segundo a porta
    char *porta = argv[2];

    //hints para a getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SCOK_STREAM; //tcp
    hints.ai_flags = AI_PASSIVE;     // usar meu IP (para bind)

    if (strcmp(protocolo, "v4") == 0) {
        hints.ai_family = AF_INET; // Forçar IPv4 [cite: 93]
    } else if (strcmp(protocolo, "v6") == 0) {
        hints.ai_family = AF_INET6; // Forçar IPv6 [cite: 93]
    } else {
        fprintf(stderr, "Protocolo inválido. Use 'v4' ou 'v6'.\n");
        exit(EXIT_FAILURE);
    }

    if ((status = getaddrinfo(NULL, porta, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    //socket e bind

    for (p = servinfo; p != NULL; p = p->ai_next) {
        // socket()
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); [cite: 94]
        if (sockfd < 0) {
            perror("servidor: ERRO socket()");
            continue;
        }

        // // 3.2. setsockopt() (Opcional, mas útil para reusar a porta em testes)
        // if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        //     perror("servidor: ERRO setsockopt()");
        //     close(sockfd);
        //     exit(1);
        // }

        // bind()
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) { [cite: 105]
            perror("servidor: ERRO bind()");
            close(sockfd);
            continue;
        }

        break; 
    }    

    freeaddrinfo(servinfo);

    //listen

    if (listen(sockfd, BACKLOG) < 0) { [cite: 182]
        perror("servidor: ERRO listen()");
        exit(1);
    }

    printf("Servidor aguardando conexões na porta %s (Protocolo: %s)...\n", porta, protocolo);

    
    //accept e conexao

    while (1) {
        cli_len = sizeof cli_addr;
        
        // accept() 
        new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len); [cite: 185]
        if (new_sockfd < 0) {
            perror("servidor: ERRO accept()");
            continue; 
        }

        inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *)&cli_addr), cli_ip_str, sizeof cli_ip_str);
        printf("Servidor: Conexão recebida de %s\n", cli_ip_str);

        //testes
        // send()  mensagem de boas-vindas
        char *msg_bem_vindo = "Conexao estabelecida com sucesso!\n";
        if (send(new_sockfd, msg_bem_vindo, strlen(msg_bem_vindo), 0) == -1) { [cite: 195]
            perror("send");
        }

        // recv() resposta do cliente
        int numbytes = recv(new_sockfd, buffer, BUFFER_SIZE - 1, 0); [cite: 208]
        if (numbytes < 0) {
            perror("recv");
        } else if (numbytes == 0) {
            printf("Servidor: Cliente (IP: %s) desconectou.\n", cli_ip_str);
        } else {
            buffer[numbytes] = '\0'; // Adiciona terminador null
            printf("Servidor: Mensagem do cliente (IP: %s): %s", cli_ip_str, buffer);
        }

      

        // close()
        close(new_sockfd); [cite: 214]
        printf("Servidor: Conexão com %s fechada. Aguardando novos clientes...\n", cli_ip_str);
    }

    close(sockfd);
    return 0;
}
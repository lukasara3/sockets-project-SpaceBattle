#include "logica.h" 

int main(int argc, char *argv[]) {

    int sockfd; // escuta
    int new_sockfd; // conexao
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage cli_addr; //conexao do cliente
    socklen_t cli_len;
    char cli_ip_str[INET6_ADDRSTRLEN]; // IP do cliente
    int status;
    int yes = 1;

    // Validar Argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <v4|v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *protocolo = argv[1];
    char *porta = argv[2];

    //hints para a getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // usar meu IP (para bind)

    if (strcmp(protocolo, "v4") == 0) {
        hints.ai_family = AF_INET; // IPv4
    } else if (strcmp(protocolo, "v6") == 0) {
        hints.ai_family = AF_INET6; // IPv6
    } else {
        fprintf(stderr, "Protocolo invalido. Use 'v4' ou 'v6'.\n");
        exit(EXIT_FAILURE);
    }

    if ((status = getaddrinfo(NULL, porta, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // socket e bind
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            perror("servidor: ERRO socket()");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("servidor: ERRO setsockopt()");
            close(sockfd);
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            perror("servidor: ERRO bind()");
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "Servidor: falha no bind.\n");
        exit(1);
    }

    // listen
    if (listen(sockfd, BACKLOG) < 0) {
        perror("servidor: ERRO listen()");
        exit(1);
    }

    printf("Servidor StarFleet aguardando conexões na porta %s...\n", porta);

    // accept e conexao (iterativo, um cliente por vez)
    while (1) {
        cli_len = sizeof cli_addr;

        new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
        if (new_sockfd < 0) {
            // Ignora erros de accept (ex: interrupção) e continua esperando
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *)&cli_addr), cli_ip_str, sizeof cli_ip_str);
        printf("Servidor: Conexão recebida de %s. Iniciando batalha.\n", cli_ip_str);

        // Processa toda a seção de jogo para este cliente
        secaoDeJogo(new_sockfd);

        // Fecha a conexão com o cliente atual
        close(new_sockfd);
        printf("Servidor: Sessão com %s terminada. Aguardando próximo cliente...\n", cli_ip_str);
    }

    close(sockfd); // Nunca será atingido
    return 0;
}
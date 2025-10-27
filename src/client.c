#include "logica.h" 

void conexao(int sockfd);
void imprimirMenu();

int main(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int status;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip/hostname> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *host = argv[1];
    char *porta = argv[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     
    hints.ai_socktype = SOCK_STREAM; // TCP

    if ((status = getaddrinfo(host, porta, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            perror("cliente: ERRO socket()");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
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

    printf("Conectado ao servidor.\n");
    printf("Sua nave: SS-42 Voyager (HP: %d)\n", HP_INICIAL);

    freeaddrinfo(servinfo); 

    conexao(sockfd);

    close(sockfd);
    return 0;
}

void conexao(int sockfd) {
    BattleMessage msg_from_server;
    BattleMessage msg_to_server;
    int game_over_signaled = 0; 

    while (!game_over_signaled) {

        int bytes_received = recv(sockfd, &msg_from_server, sizeof(BattleMessage), 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) printf("\nServidor encerrou a conexão.\n");
            else perror("recv");
            break; 
        }

        switch(msg_from_server.type) {

            case MSG_ACTION_REQ: 
                printf("%s", msg_from_server.message); 
                imprimirMenu(); 

                int acao_escolhida = -1;
                while (scanf("%d", &acao_escolhida) != 1 || acao_escolhida < 0 || acao_escolhida > 4) {
                    printf("Erro: escolha invalida!\n"); //
                    printf("\n Por favor selecione um valor entre 0 e 4.\n"); //
                    while (getchar() != '\n');
                    imprimirMenu(); 
                }
                while (getchar() != '\n');

                memset(&msg_to_server, 0, sizeof(BattleMessage));
                msg_to_server.type = MSG_ACTION_RES; 
                msg_to_server.client_action = acao_escolhida;

                if (send(sockfd, &msg_to_server, sizeof(BattleMessage), 0) == -1) {
                    perror("send action_res");
                    game_over_signaled = 1; 
                } else {
                    printf("%s\n", stringAcao(acao_escolhida, 1)); 
                }
                break;

            case MSG_BATTLE_RESULT: 
                printf("%s", msg_from_server.message);
                printf("Placar: Voce %d x %d Inimigo.\n",
                    msg_from_server.client_hp,
                    msg_from_server.server_hp);
                break;

            case MSG_ESCAPE:   
            case MSG_GAME_OVER:
                printf("%s", msg_from_server.message);
                break;

            case MSG_INVENTORY: 
                printf("%s", msg_from_server.message);
                game_over_signaled = 1; 
                break;

            default:
                printf("Mensagem desconhecida recebida do servidor: type %d\n", msg_from_server.type);
        }
    } 
}

void imprimirMenu() {
    printf("Escolha sua acao:\n"); 
    printf("0 - Laser Attack\n");     
    printf("1 - Photon Torpedo\n"); 
    printf("2 - Shields Up\n");     
    printf("3 - Cloaking\n");      
    printf("4 - Hyper Jump\n");     
}
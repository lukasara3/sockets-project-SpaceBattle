#include "logica.h"

static const char* MSG_S_LASER = "Servidor disparou um Laser!\n";
static const char* MSG_S_TORPEDO = "Servidor disparou um Photon Torpedo!\n";
static const char* MSG_S_SHIELDS = "Servidor ativou os Escudos!\n";
static const char* MSG_S_CLOAKING = "Servidor usou Cloaking.\n";
static const char* MSG_S_HYPER_JUMP = "Servidor acionou o Hyper Jump!\n";

static const char* RES_DANO_CLIENTE = "Resultado: Voce recebeu 20 de dano.\n";
static const char* RES_DANO_SERVIDOR = "Resultado: Nave inimiga perdeu 20 HP.\n";
static const char* RES_DANO_AMBOS = "Resultado: Ambos receberam 20 de dano.\n";
static const char* RES_BLOQUEIO_CLIENTE = "Resultado: Ataque inimigo bloqueado!\n";
static const char* RES_BLOQUEIO_SERVIDOR = "Resultado: Ataque bloqueado pelos escudos inimigos!\n";
static const char* RES_EVASAO_CLIENTE = "Resultado: Ataque inimigo falhou!\n";
static const char* RES_EVASAO_SERVIDOR = "Resultado: Voce errou o Torpedo (inimigo camuflado)!\n";
static const char* RES_SEM_DANO = "Resultado: Ninguem recebeu dano.\n";

static TurnResult resolveTurno(int client_act, int server_act, int* torp_used, int* shld_used);

void secaoDeJogo(int sockfd) {

    int client_hp = HP_INICIAL;
    int server_hp = HP_INICIAL;
    int client_torpedoes_used = 0;
    int client_shields_used = 0;
    int turn_count = 0;
    int game_over = 0;
    int client_action = -1;
    int server_action = -1;
    MessageType final_status_type = MSG_GAME_OVER;
    char status_msg[100] = {0};

    srand(time(NULL) ^ getpid());

    BattleMessage msg_to_client;
    BattleMessage msg_from_client;

    while (!game_over) {
        turn_count++;

        memset(&msg_to_client, 0, sizeof(BattleMessage));
        msg_to_client.type = MSG_ACTION_REQ;
        sprintf(msg_to_client.message, "Turno %d. HP: Voce(%d) x Inimigo(%d)\n", turn_count, client_hp, server_hp);
        if (send(sockfd, &msg_to_client, sizeof(BattleMessage), 0) == -1) {
            perror("send action_req");
            strcpy(status_msg, "Erro de comunicação com o cliente.");
            final_status_type = MSG_GAME_OVER;
            break;
        }

        int bytes_received = recv(sockfd, &msg_from_client, sizeof(BattleMessage), 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) printf("Cliente desconectou durante o turno.\n");
            else perror("recv action_res");
            game_over = 1;
            client_hp = 0;
            strcpy(status_msg, "A batalha terminou por desconexao.");
            final_status_type = MSG_GAME_OVER;
            continue;
        }
        client_action = msg_from_client.client_action;

        server_action = rand() % 5;

        TurnResult turn_res = resolveTurno(client_action, server_action, &client_torpedoes_used, &client_shields_used);
        client_hp -= turn_res.client_damage_taken;
        server_hp -= turn_res.server_damage_taken;

        memset(&msg_to_client, 0, sizeof(BattleMessage));
        msg_to_client.type = MSG_BATTLE_RESULT;
        msg_to_client.client_action = client_action;
        msg_to_client.server_action = server_action;
        msg_to_client.client_hp = client_hp;
        msg_to_client.server_hp = server_hp;
        strncpy(msg_to_client.message, turn_res.text, MSG_SIZE - 1);
        if (send(sockfd, &msg_to_client, sizeof(BattleMessage), 0) == -1) {
            perror("send battle_result");
            strcpy(status_msg, "Erro de comunicação com o cliente.");
            final_status_type = MSG_GAME_OVER;
            break;
        }

        if (client_action == HYPER_JUMP || server_action == HYPER_JUMP) {
            game_over = 1;
            final_status_type = MSG_ESCAPE;
            if (client_action == HYPER_JUMP && server_action == HYPER_JUMP) strcpy(status_msg, "Fuga mutua. A batalha terminou.");
            else if (client_action == HYPER_JUMP) strcpy(status_msg, "Voce escapou para o hiperespaco.");
            else strcpy(status_msg, "O inimigo escapou para o hiperespaco.");
        } else if (client_hp <= 0 || server_hp <= 0) {
            game_over = 1;
            final_status_type = MSG_GAME_OVER;
            if (client_hp <= 0) strcpy(status_msg, "Sua nave foi destruida!");
            else strcpy(status_msg, "Voce derrotou a frota inimiga!");
        }
    }

    if (client_hp < 0) client_hp = 0;
    if (server_hp < 0) server_hp = 0;

    memset(&msg_to_client, 0, sizeof(BattleMessage));
    msg_to_client.type = final_status_type;
    snprintf(msg_to_client.message, MSG_SIZE, "Fim de jogo!\n");
    msg_to_client.client_hp = client_hp;
    msg_to_client.server_hp = server_hp;
    send(sockfd, &msg_to_client, sizeof(BattleMessage), 0);

    memset(&msg_to_client, 0, sizeof(BattleMessage));
    msg_to_client.type = MSG_INVENTORY;
    char inventory_message[MSG_SIZE * 4];
    sprintf(inventory_message,
        "Inventario final:\n"
        "- HP restante: %d\n"
        "- HP inimigo restante: %d\n"
        "- Total de turnos jogados: %d\n"
        "- Torpedos usados: %d\n"
        "- Escudos usados: %d\n",
        client_hp, server_hp, turn_count, client_torpedoes_used, client_shields_used
    );
    size_t len = strlen(status_msg);
    if (len > 0 && status_msg[len - 1] == '\n') {
        status_msg[len - 1] = '\0';
    }
    strncat(inventory_message, status_msg, sizeof(inventory_message) - strlen(inventory_message) - 1);
    strncat(inventory_message, "\n", sizeof(inventory_message) - strlen(inventory_message) - 1);
    
    if (final_status_type == MSG_ESCAPE) {
        strncat(inventory_message, "Obrigado por jogar!\n", sizeof(inventory_message) - strlen(inventory_message) - 1); // Garante \n no final
    }

    msg_to_client.client_hp = client_hp;
    msg_to_client.server_hp = server_hp;
    msg_to_client.client_torpedoes = client_torpedoes_used;
    msg_to_client.client_shields = client_shields_used;
    strncpy(msg_to_client.message, inventory_message, MSG_SIZE - 1);
    msg_to_client.message[MSG_SIZE - 1] = '\0';
    send(sockfd, &msg_to_client, sizeof(BattleMessage), 0);
}


static TurnResult resolveTurno(int client_act, int server_act, int* torp_used, int* shld_used) {
    TurnResult res;
    memset(&res, 0, sizeof(TurnResult));
    char text_buffer[MSG_SIZE * 2] = {0};

    const char* s_str = stringAcao(server_act, 0);
    strncat(text_buffer, s_str, sizeof(text_buffer) - strlen(text_buffer) - 1);

    if (client_act == TORPEDO) (*torp_used)++;
    if (client_act == SHIELDS) (*shld_used)++;

    if (client_act == HYPER_JUMP || server_act == HYPER_JUMP) {
        strncat(text_buffer, RES_SEM_DANO, sizeof(text_buffer) - strlen(text_buffer) - 1);
        strncpy(res.text, text_buffer, sizeof(res.text) - 1);
        res.text[sizeof(res.text) - 1] = '\0';
        return res;
    }

    int client_is_shielded = (client_act == SHIELDS);
    int server_is_shielded = (server_act == SHIELDS);
    int client_is_cloaked = (client_act == CLOAKING);
    int server_is_cloaked = (server_act == CLOAKING);

    if (client_act == TORPEDO && server_act == LASER) {
        res.server_damage_taken = DANO_ATAQUE;
        strncat(text_buffer, RES_DANO_SERVIDOR, sizeof(text_buffer) - strlen(text_buffer) - 1);
    }
    else if (client_act == LASER && server_act == TORPEDO) {
        res.client_damage_taken = DANO_ATAQUE;
        strncat(text_buffer, RES_DANO_CLIENTE, sizeof(text_buffer) - strlen(text_buffer) - 1);
    }
    else if (client_act == LASER && server_act == LASER) {
        res.client_damage_taken = DANO_ATAQUE;
        res.server_damage_taken = DANO_ATAQUE;
        strncat(text_buffer, RES_DANO_AMBOS, sizeof(text_buffer) - strlen(text_buffer) - 1);
    }
    else if (client_act == TORPEDO && server_act == TORPEDO) {
        res.client_damage_taken = DANO_ATAQUE;
        res.server_damage_taken = DANO_ATAQUE;
        strncat(text_buffer, RES_DANO_AMBOS, sizeof(text_buffer) - strlen(text_buffer) - 1);
    }
    else {
        int client_damage = 0;
        int server_damage = 0;
        int client_blocked = 0;
        int server_blocked = 0;
        int client_evaded = 0;
        int server_evaded = 0;

        if (client_act == LASER) {
            if (server_is_shielded) server_blocked = 1;
            else server_damage = DANO_ATAQUE;
        }
        else if (client_act == TORPEDO) {
            if (server_is_shielded) server_blocked = 1;
            else if (server_is_cloaked) server_evaded = 1;
            else server_damage = DANO_ATAQUE;
        }

        if (server_act == LASER) {
            if (client_is_shielded) client_blocked = 1;
            else client_damage = DANO_ATAQUE;
        }
        else if (server_act == TORPEDO) {
            if (client_is_shielded) client_blocked = 1;
            else if (client_is_cloaked) client_evaded = 1;
            else client_damage = DANO_ATAQUE;
        }

        res.client_damage_taken = client_damage;
        res.server_damage_taken = server_damage;

        if (client_damage > 0 && server_damage > 0) strncat(text_buffer, RES_DANO_AMBOS, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (client_damage > 0) strncat(text_buffer, RES_DANO_CLIENTE, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (server_damage > 0) strncat(text_buffer, RES_DANO_SERVIDOR, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (client_evaded) strncat(text_buffer, RES_EVASAO_CLIENTE, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (server_evaded) strncat(text_buffer, RES_EVASAO_SERVIDOR, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (client_blocked && server_blocked) strncat(text_buffer, RES_SEM_DANO, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (client_blocked) strncat(text_buffer, RES_BLOQUEIO_CLIENTE, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else if (server_blocked) strncat(text_buffer, RES_BLOQUEIO_SERVIDOR, sizeof(text_buffer) - strlen(text_buffer) - 1);
        else strncat(text_buffer, RES_SEM_DANO, sizeof(text_buffer) - strlen(text_buffer) - 1);
    }

    strncpy(res.text, text_buffer, sizeof(res.text) - 1);
    res.text[sizeof(res.text) - 1] = '\0';
    return res;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

const char* stringAcao(int action, int is_client) {
    if (is_client) {
        switch(action) {
            case LASER: return "Voce disparou um Laser!";
            case TORPEDO: return "Voce disparou um Photon Torpedo!";
            case SHIELDS: return "Voce ativou os Escudos!";
            case CLOAKING: return "Voce ativou Cloaking!";
            case HYPER_JUMP: return "Voce acionou o Hyper Jump!";
            default: return "Acao Invalida.";
        }
    } else {
        switch(action) {
            case LASER: return MSG_S_LASER;
            case TORPEDO: return MSG_S_TORPEDO;
            case SHIELDS: return MSG_S_SHIELDS;
            case CLOAKING: return MSG_S_CLOAKING;
            case HYPER_JUMP: return MSG_S_HYPER_JUMP;
            default: return "Servidor tomou uma acao invalida.\n";
        }
    }
}
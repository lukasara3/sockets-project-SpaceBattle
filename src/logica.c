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


// funções estaticas
static TurnResult ResolveTurno(int client_act, int server_act, int* torp_used, int* shld_used);

void SecaoDeJogo(int sockfd) {
    
    int client_hp = HP_INICIAL;
    int server_hp = HP_INICIAL;
    int client_torpedoes_used = 0;
    int client_shields_used = 0;
    int turn_count = 0;
    int game_over = 0;
    int client_action = -1; // checar fuga
    int server_action = -1;

    // gerador aleatório
    srand(time(NULL) ^ getpid()); 

    BattleMessage msg_to_client;
    BattleMessage msg_from_client;
    
    while (!game_over) {
        turn_count++;

        memset(&msg_to_client, 0, sizeof(BattleMessage));
        msg_to_client.type = MSG_ACTION_REQ;
        sprintf(msg_to_client.message, "Turno %d. HP: Voce(%d) | Inimigo(%d)\n", turn_count, client_hp, server_hp);
        
        if (send(sockfd, &msg_to_client, sizeof(BattleMessage), 0) == -1) {
            perror("send action_req");
            break; 
        }

        int bytes_received = recv(sockfd, &msg_from_client, sizeof(BattleMessage), 0);
        if (bytes_received <= 0) {
            printf("Cliente desconectou duringo o turno.\n");
            game_over = 1; 
            client_hp = 0; 
            continue; 
        }
        
        client_action = msg_from_client.client_action;

        server_action = rand() % 5; 

        TurnResult turn_res = ResolveTurno(client_action, server_action, &client_torpedoes_used, &client_shields_used);
        
        client_hp -= turn_res.client_damage_taken;
        server_hp -= turn_res.server_damage_taken;

        memset(&msg_to_client, 0, sizeof(BattleMessage));
        msg_to_client.type = MSG_BATTLE_RESULT;
        msg_to_client.client_action = client_action;
        msg_to_client.server_action = server_action;
        msg_to_client.client_hp = client_hp;
        msg_to_client.server_hp = server_hp;
        strcpy(msg_to_client.message, turn_res.text); 
        
        if (send(sockfd, &msg_to_client, sizeof(BattleMessage), 0) == -1) {
            perror("send battle_result");
            break; 
        }

        if (client_hp <= 0 || server_hp <= 0 || client_action == HYPER_JUMP || server_action == HYPER_JUMP) {
            game_over = 1;
        }
    } 
    
    memset(&msg_to_client, 0, sizeof(BattleMessage));
    msg_to_client.type = MSG_GAME_OVER;
    
    char final_message[MSG_SIZE * 4]; 
    char status_msg[100];
    
    if (client_action == HYPER_JUMP && server_action == HYPER_JUMP) strcpy(status_msg, "Fuga mutua. A batalha terminou.");
    else if (client_action == HYPER_JUMP) strcpy(status_msg, "Voce escapou para o hiperespaco.");
    else if (server_action == HYPER_JUMP) strcpy(status_msg, "O inimigo escapou para o hiperespaco.");
    else if (client_hp <= 0) strcpy(status_msg, "Sua nave foi destruida!");
    else if (server_hp <= 0) strcpy(status_msg, "Voce derrotou a frota inimiga!");
    else strcpy(status_msg, "A batalha terminou por desconexao.");

    if (client_hp < 0) client_hp = 0;
    if (server_hp < 0) server_hp = 0;
    
    sprintf(final_message,
        "Fim de jogo!\n"
        "%s\n\n"
        "--- Inventario final ---\n"
        "HP restante: %d\n"
        "HP inimigo restante: %d\n" 
        "Total de turnos jogados: %d\n"
        "Torpedos usados: %d\n"
        "Escudos usados: %d\n",
        status_msg, client_hp, server_hp, turn_count, client_torpedoes_used, client_shields_used
    );
    
    msg_to_client.client_hp = client_hp;
    msg_to_client.server_hp = server_hp;
    msg_to_client.client_torpedoes = client_torpedoes_used;
    msg_to_client.client_shields = client_shields_used;
    strcpy(msg_to_client.message, final_message);
    
    send(sockfd, &msg_to_client, sizeof(BattleMessage), 0);
}


static TurnResult ResolveTurno(int client_act, int server_act, int* torp_used, int* shld_used) {
    TurnResult res;
    memset(&res, 0, sizeof(TurnResult));
    char text_buffer[MSG_SIZE * 2] = {0};

    const char* s_str = StringAcao(server_act, 0); 
    strcat(text_buffer, s_str);

    if (client_act == TORPEDO) (*torp_used)++;
    if (client_act == SHIELDS) (*shld_used)++;

    if (client_act == HYPER_JUMP || server_act == HYPER_JUMP) {
        strcat(text_buffer, RES_SEM_DANO); 
        strcpy(res.text, text_buffer);
        return res; 
    }

    int client_is_shielded = (client_act == SHIELDS);
    int server_is_shielded = (server_act == SHIELDS);
    int client_is_cloaked = (client_act == CLOAKING);
    int server_is_cloaked = (server_act == CLOAKING);
    
    if (client_act == TORPEDO && server_act == LASER) {
        res.server_damage_taken = DANO_ATAQUE; 
        strcat(text_buffer, RES_DANO_SERVIDOR);
    } 
    else if (client_act == LASER && server_act == TORPEDO) {
        res.client_damage_taken = DANO_ATAQUE; 
        strcat(text_buffer, RES_DANO_CLIENTE);
    }
    else if (client_act == LASER && server_act == LASER) {
        res.client_damage_taken = DANO_ATAQUE;
        res.server_damage_taken = DANO_ATAQUE;
        strcat(text_buffer, RES_DANO_AMBOS);
    }
    else if (client_act == TORPEDO && server_act == TORPEDO) {
        res.client_damage_taken = DANO_ATAQUE;
        res.server_damage_taken = DANO_ATAQUE;
        strcat(text_buffer, RES_DANO_AMBOS);
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
        
        if (client_damage > 0 && server_damage > 0) strcat(text_buffer, RES_DANO_AMBOS); 
        else if (client_damage > 0) strcat(text_buffer, RES_DANO_CLIENTE);
        else if (server_damage > 0) strcat(text_buffer, RES_DANO_SERVIDOR);
        else if (client_evaded) strcat(text_buffer, RES_EVASAO_CLIENTE);
        else if (server_evaded) strcat(text_buffer, RES_EVASAO_SERVIDOR);
        else if (client_blocked && server_blocked) strcat(text_buffer, RES_SEM_DANO); 
        else if (client_blocked) strcat(text_buffer, RES_BLOQUEIO_CLIENTE);
        else if (server_blocked) strcat(text_buffer, RES_BLOQUEIO_SERVIDOR);
        else strcat(text_buffer, RES_SEM_DANO); 
    }

    strcpy(res.text, text_buffer);
    return res;
}

const char* StringAcao(int action, int is_client) {
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
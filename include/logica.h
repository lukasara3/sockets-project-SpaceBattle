#ifndef LOGICA_H
#define LOGICA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> 
#include <time.h>   
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h> 

#define MSG_SIZE 256
#define HP_INICIAL 100
#define DANO_ATAQUE 20 
#define BACKLOG 10 

typedef enum {
    MSG_INIT,        // 0 
    MSG_ACTION_REQ,  // 1
    MSG_ACTION_RES,  // 2
    MSG_BATTLE_RESULT, // 3
    MSG_GAME_OVER    // 4 
} MessageType;

typedef enum {
    LASER,        // 0
    TORPEDO,      // 1
    SHIELDS,      // 2
    CLOAKING,     // 3
    HYPER_JUMP    // 4
} ActionType;

typedef struct {
    int type;
    int client_action;
    int server_action;
    int client_hp;
    int server_hp;
    int client_torpedoes;
    int client_shields;
    char message[MSG_SIZE];
} BattleMessage;

typedef struct {
    int client_damage_taken;
    int server_damage_taken;
    char text[MSG_SIZE * 2];
} TurnResult;

void SecaoDeJogo(int sockfd); 
const char* StringAcao(int action, int is_client);
void *get_in_addr(struct sockaddr *sa);

#endif 
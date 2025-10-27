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
    MSG_INIT,
    MSG_ACTION_REQ, 
    MSG_ACTION_RES,
    MSG_BATTLE_RESULT, 
    MSG_INVENTORY, 
    MSG_GAME_OVER, 
    MSG_ESCAPE 
} MessageType ;

typedef enum {
    LASER,        
    TORPEDO,      
    SHIELDS,      
    CLOAKING,     
    HYPER_JUMP    
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

void secaoDeJogo(int sockfd); 
const char* stringAcao(int action, int is_client);
void *get_in_addr(struct sockaddr *sa);
#endif 
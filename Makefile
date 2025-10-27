CC = gcc
CFLAGS = -Wall -g -std=c11 -Iinclude -D_DEFAULT_SOURCE

BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

SERVER_SRCS = $(SRC_DIR)/servidor.c $(SRC_DIR)/logica.c
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

CLIENT_SRCS = $(SRC_DIR)/client.c $(SRC_DIR)/logica.c
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

SERVER_BIN = $(BIN_DIR)/server
CLIENT_BIN = $(BIN_DIR)/client


HEADER = $(INCLUDE_DIR)/logica.h

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Servidor linkado: $@"

$(CLIENT_BIN): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Cliente linkado: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ 
	@echo "Compilado: $<"

clean:
	@rm -f $(SERVER_BIN) $(CLIENT_BIN)
	@rm -rf $(OBJ_DIR)
	@echo "Binários e objetos removidos."

.PHONY: all clean
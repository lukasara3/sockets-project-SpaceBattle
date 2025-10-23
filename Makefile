CC = gcc
CFLAGS = -Wall -g -std=c11

SRC_DIR = src
BIN_DIR = bin

CLIENT_SRC = $(SRC_DIR)/cliente.c
SERVER_SRC = $(SRC_DIR)/servidor.c

CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC)
	@mkdir -p $(BIN_DIR) 
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)
	@echo "Cliente compilado em $(CLIENT_BIN)"

$(SERVER_BIN): $(SERVER_SRC)
	@mkdir -p $(BIN_DIR) 
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)
	@echo "Servidor compilado em $(SERVER_BIN)"

clean:
	@rm -f $(BIN_DIR)/client $(BIN_DIR)/server
	@echo "Binários removidos."

.PHONY: all clean
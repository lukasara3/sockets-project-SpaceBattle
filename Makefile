# Compilador
CC = gcc

# Flags: -Wall (warnings), -g (debug), -std=c11
# -Iinclude (Procure headers na pasta 'include/')
# -D_DEFAULT_SOURCE (Para expor SA_RESTART e outras)
CFLAGS = -Wall -g -std=c11 -Iinclude -D_DEFAULT_SOURCE

# --- Diretórios ---
BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# --- Fontes (.c) e Objetos (.o) ---
# Fontes do Servidor
SERVER_SRCS = $(SRC_DIR)/servidor.c $(SRC_DIR)/logica.c
# Objetos do Servidor
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

# --- CORREÇÃO AQUI ---
# Fontes do Cliente (agora inclui logica.c)
CLIENT_SRCS = $(SRC_DIR)/client.c $(SRC_DIR)/logica.c
# Objetos do Cliente (derivados das fontes)
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))
# --- FIM DA CORREÇÃO ---

# --- Binários ---
SERVER_BIN = $(BIN_DIR)/server
CLIENT_BIN = $(BIN_DIR)/client

# --- Header (Dependência) ---
# Header principal
HEADER = $(INCLUDE_DIR)/logica.h

# --- Regras ---

# Regra Padrão: 'make' ou 'make all'
all: $(SERVER_BIN) $(CLIENT_BIN)

# Regra para LINKAR o executável do servidor
$(SERVER_BIN): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Servidor linkado: $@"

# Regra para LINKAR o executável do cliente (Corrigida para CLIENT_OBJS)
$(CLIENT_BIN): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Cliente linkado: $@"

# Regra Padrão para COMPILAR .c para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ 
	@echo "Compilado: $<"

# Regra para Limpar
clean:
	@rm -f $(SERVER_BIN) $(CLIENT_BIN)
	@rm -rf $(OBJ_DIR)
	@echo "Binários e objetos removidos."

# Regras que não geram arquivos
.PHONY: all clean
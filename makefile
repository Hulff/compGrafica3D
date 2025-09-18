# Nome do executável
TARGET = game

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O2 -Iinclude

# Bibliotecas necessárias
LIBS = -lGL -lGLU -lglut

# Diretórios
SRC_DIR = src
OBJ_DIR = build

# Todos os arquivos .c (raiz + src e subpastas)
SRCS = $(shell find $(SRC_DIR) -name '*.c') $(wildcard *.c)

# Arquivos objeto em build/
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

# Regra padrão
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compilação: build/espelho das pastas
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Executar
run: all
	./$(TARGET)

# Limpar
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild
rebuild: clean all

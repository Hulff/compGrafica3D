# Nome do executável
TARGET = game

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O2 -Iinclude

# Diretórios
SRC_DIR = src
OBJ_DIR = build

# Detecta sistema operacional
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LIBS = -lGL -lGLU -lglut -lm
    RUN_CMD = ./$(TARGET)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    LIBS = -lfreeglut -lopengl32 -lglu32 -lm
    RUN_CMD = ./$(TARGET).exe
endif

# Todos os arquivos .c (raiz + src e subpastas)
SRCS = $(wildcard *.c) $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c)

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
	$(RUN_CMD)

# Limpar
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET).exe

# Rebuild
rebuild: clean all
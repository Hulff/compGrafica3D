# Nome do executável
TARGET = game

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O2 -Iinclude

# Bibliotecas necessárias (OpenGL, GLU e GLUT)
LIBS = -lGL -lGLU -lglut

# Diretórios de código-fonte
SRC_DIRS = . helpers operations shape

# Arquivos fonte (.c) em todos os diretórios
SRCS = $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

# Arquivos objeto correspondentes (.o)
OBJS = $(SRCS:.c=.o)

# Regra padrão (gera o executável)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Regra para compilar cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Executar o jogo
run: all
	./$(TARGET)

# Limpar arquivos temporários
clean:
	rm -f $(OBJS) $(TARGET)

# Recompilar do zero
rebuild: clean all

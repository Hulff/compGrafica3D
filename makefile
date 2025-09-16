# Nome do executável
TARGET = game

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O2

# Bibliotecas necessárias (OpenGL, GLU e GLUT)
LIBS = -lGL -lGLU -lglut

# Arquivos fonte (.c)
SRCS = main.c \
       game.c \
       render.c \
       camera.c \
       utils.c

# Arquivos objeto (.o) gerados automaticamente
OBJS = $(SRCS:.c=.o)

# Regra padrão (gera o jogo)
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

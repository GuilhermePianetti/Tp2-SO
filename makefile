# Compilador e flags
CC = gcc
CFLAGS = -Wall -g

# Arquivos-fonte e cabeçalhos
SRCS = main.c menu.c diretorio.c arquivo.c particao.c modoAutomatico.c
OBJS = $(SRCS:.c=.o)

# Nome do executável
EXEC = sistema_arquivos

# Regra padrão
all: $(EXEC)

# Como compilar o executável
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Regra para limpar os arquivos objetos e o executável
clean:
	rm -f $(OBJS) $(EXEC)

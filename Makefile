CC = gcc
CFLAGS = -Wall -I./Modulos/Arquivo -I./Modulos/Diretorio -I./Modulos/Menu -I./Modulos/Outros -I./Modulos/Particao

SRC = main.c \
      Modulos/Arquivo/arquivo.c \
      Modulos/Diretorio/diretorio.c \
      Modulos/Menu/menu.c \
      Modulos/Menu/modoAutomatico.c \
      Modulos/Outros/caminho.c \
      Modulos/Particao/particao.c


OBJ = $(SRC:.c=.o)
EXEC = programa

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

clean:
	rm -f $(OBJ) $(EXEC)

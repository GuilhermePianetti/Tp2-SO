#ifndef GLOBAIS_H
#define GLOBAIS_H

#include "particao.h"
#include <stdbool.h>

extern particao sistema;
extern int diretorio_atual;
extern bool sistema_inicializado;
extern bool modo_verboso;

#ifndef CONSTANTES_H
#define CONSTANTES_H

#define MAX_NOME 63
#define MAX_CAMINHO_DEPTH 32
#define MAX_CAMINHO_STR 1024
#define MAX_LINHA_COMANDO 512
#define MAX_ARGS 10
#define MAX_ARG_LENGTH 256

#endif
#endif

// modoAutomatico.h

#ifndef MODO_AUTOMATICO_H
#define MODO_AUTOMATICO_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "menu.h"

#define MAX_LINHA_COMANDO 512
#define MAX_ARGS 10
#define MAX_ARG_LENGTH 256

typedef struct {
    char comando[MAX_ARG_LENGTH];
    char args[MAX_ARGS][MAX_ARG_LENGTH];
    int num_args;
} comando_automatico;

// Declarações das variáveis globais (não definidas aqui!)
extern bool modo_automatico;
extern FILE* arquivo_comandos;
extern int linha_atual;
extern bool pausar_em_erro;
extern bool mostrar_comandos;

// Funções
void trim(char* str);
int parsearComando(const char* linha, comando_automatico* cmd);
int executarCriarDir(comando_automatico* cmd);
int executarCriarArquivo(comando_automatico* cmd);
int executarNavegar(comando_automatico* cmd);
int executarListar(comando_automatico* cmd);
int executarImportar(comando_automatico* cmd);
int executarRenomear(comando_automatico* cmd);
int executarMover(comando_automatico* cmd);
int executarApagar(comando_automatico* cmd);
int executarInfo(comando_automatico* cmd);
int executarEcho(comando_automatico* cmd);
int executarPausar(comando_automatico* cmd);
int executarComandoAutomatico(comando_automatico* cmd);
int executarArquivoComandos(const char* caminho_arquivo);
void menuExecucaoAutomatica(void);


#endif

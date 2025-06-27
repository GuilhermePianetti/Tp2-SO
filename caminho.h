#ifndef CAMINHO_H
#define CAMINHO_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#include "particao.h"
#include "diretorio.h"
#include "arquivo.h"
#include "modoAutomatico.h"
#include "globais.h"

typedef struct {
    int inode;
    char nome[MAX_NOME + 1];
} nivel_caminho;

// Constantes
#define MAX_CAMINHO_DEPTH 32
#define MAX_CAMINHO_STR 1024
#define MAX_NOME 63

// Variáveis globais
extern nivel_caminho caminho_atual[MAX_CAMINHO_DEPTH];
extern int profundidade_atual;
extern char caminho_string[MAX_CAMINHO_STR];

// Funções
void limparBuffer(void);
void pausar(void);

char* obterNomeDiretorio(particao* p, int inode_alvo);
void construirCaminhoString(void);
void exibirCaminhoAtual(void);

int navegarPara(particao* p, const char* nome_diretorio);
void inicializarSistemaCaminho(void);
void irParaRaiz(void);
const char* obterCaminhoAtual(void);

void menuNavegacao(void);
void exibirCaminhoComBreadcrumb(void);

int resolverCaminhoCompleto(particao* p, const char* caminho);
bool verificarCaminhoEDiretorio(particao* p, const char* caminho);
void mostrarSugestoesCaminhos(particao* p, int inode_base, const char* prefixo_caminho);

int moverArquivoComCaminho(particao* p, int inode_dir_origem, const char* nome_arquivo, const char* caminho_destino);
int navegarParaCaminhoCompleto(particao* p, const char* caminho);

void menuMoverArquivoAtualizado(void);
void menuNavegacaoAvancada(void);

// Dependência externa
int buscarEntradaDiretorio(particao* p, int inode_dir, const char* nome_entrada);
int moverArquivo(particao* p, int inode_origem, const char* nome_arquivo, int inode_destino);

#endif // CAMINHO_H

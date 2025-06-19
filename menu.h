#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "particao.h"
#include "diretorio.h"
#include "arquivo.h"
#include "modoAutomatico.h"

// Constantes para caminho
#define MAX_CAMINHO_DEPTH 50
#define MAX_CAMINHO_STR 1024

// Estrutura para representar um nível do caminho
typedef struct {
    int inode;
    char nome[MAX_NOME + 1];
} nivel_caminho;

// Variáveis globais externas
extern particao sistema;
extern int diretorio_atual;
extern bool sistema_inicializado;
extern bool modo_verboso;

// Variáveis globais relacionadas ao caminho
extern nivel_caminho caminho_atual[MAX_CAMINHO_DEPTH];
extern int profundidade_atual;
extern char caminho_string[MAX_CAMINHO_STR];

// Utilitários
void limparBuffer(void);
void pausar(void);
void construirCaminhoString();

// Caminho e navegação
void exibirCaminhoAtual(void);
const char* obterCaminhoAtual(void);
char* obterNomeDiretorio(particao* p, int inode_alvo);
void inicializarSistemaCaminho(void);
int navegarPara(particao* p, const char* nome_diretorio);
void irParaRaiz(void);
void menuNavegacao(void);
void exibirCaminhoComBreadcrumb(void);

// Menus principais
void menuPrincipal(void);
void menuInicializarSistema(void);
void menuDiretorios(void);
void menuArquivos(void);
void menuConfiguracoes(void);

int resolverCaminhoCompleto(particao* p, const char* caminho);
bool verificarCaminhoEDiretorio(particao* p, const char* caminho);
void mostrarSugestoesCaminhos(particao* p, int inode_base, const char* prefixo_caminho);
int moverArquivoComCaminho(particao* p, int inode_dir_origem, const char* nome_arquivo, const char* caminho_destino);
int navegarParaCaminhoCompleto(particao* p, const char* caminho);
void menuMoverArquivoAtualizado();
void menuNavegacaoAvancada();

#endif // MENU_H

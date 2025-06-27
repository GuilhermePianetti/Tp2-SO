#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../Particao/particao.h"
#include "../Diretorio/diretorio.h"
#include "../Menu/menu.h"



// Criação
int criarArquivo(particao *p, const char *nome, int inode_pai);

// Escrita e leitura
int escreverArquivo(particao *p, int inode_arquivo, const char *dados, int tamanho);
void listarConteudoArquivo(particao *p, int inode_arquivo);
int importarArquivo(particao *p, int inode_arquivo, const char *caminho_arquivo_real);

// Renomear, mover e apagar
int renomearArquivo(particao *p, int inode_dir, const char *nome_antigo, const char *nome_novo);
int moverArquivo(particao *p, int inode_dir_origem, const char *nome_arquivo, int inode_dir_destino);
int apagarArquivo(particao *p, int inode_dir, const char *nome_arquivo);

// Busca e informações
int buscarArquivoRecursivo(particao *p, int inode_dir, const char *nome_arquivo);
void informacoesArquivo(particao *p, int inode_arquivo);

#endif // ARQUIVOS_H

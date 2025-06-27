#ifndef DIRETORIO_H
#define DIRETORIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../Particao/particao.h"
#include "../Arquivo/arquivo.h"
#include "../Menu/menu.h"


// Criação e remoção de diretórios
int criarDiretorio(particao *p, const char *nome, int inode_pai);
int apagarDiretorio(particao *p, int inode_dir_pai, const char *nome);

// Operações em entradas de diretório
int buscarEntradaDiretorio(particao *p, int inode_dir, const char *nome);
int adicionarEntradaDiretorio(particao *p, int inode_dir, const char *nome, int inode_arquivo);
int renomearEntrada(particao *p, int inode_dir, const char *nome_antigo, const char *nome_novo);

#endif // DIRETORIO_H

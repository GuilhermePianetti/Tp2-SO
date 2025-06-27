#ifndef PARTICAO_H
#define PARTICAO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>



#define NUM_PONTEIROS_DIRETOS 12
#define MAX_ENTRADAS_POR_BLOCO 16
#define MAX_NOME 63 

typedef struct inode {
    int numero;                       // Número do i-node
    int tipo;                         // 0 = arquivo, 1 = diretório, -1 = livre
    int tamanho;                      // Tamanho do arquivo em bytes
    time_t data_criacao;                
    time_t data_modificacao;
    time_t data_acesso;
    int blocos_diretos[NUM_PONTEIROS_DIRETOS];
} inode;

typedef struct entrada_diretorio {
    char nome[MAX_NOME + 1];         // Nome do arquivo ou diretório
    int numero_inode;                // Número do i-node associado
    int valida;                      // 1 = válida, 0 = removida
} entrada_diretorio;

typedef struct particao {
    int tamanho;                    // Tamanho total da partição
    int tamanhoBloco;               // Tamanho de cada bloco
    int numBlocos;                  // Quantidade total de blocos
    int numInodes;                  // Quantidade de i-nodes
    int *bitmap;                    // Bitmap de blocos
    int *bitmapInodes;              // Bitmap de i-nodes
    inode *inodes;                  // Vetor de i-nodes
    char **blocos;                  // Vetor de ponteiros para blocos
    int inode_raiz;                 // I-node do diretório raiz
} particao;

// Funções de inicialização e gerenciamento de partição
int criarDiretorioRaiz(particao *p);
int inicializarParticao(particao *p, int tamanho, int tamanhoBloco);
void liberarParticao(particao *p);

// Funções de alocação
int encontrarInodeLivre(particao *p);
int encontrarBlocoLivre(particao *p);
int encontrarOuAlocarBloco(particao *p, int inode_num, int tipo_conteudo);

// Utilitários e visualização
void listarDiretorio(particao *p, int inode_dir);
void mostrarEstatisticas(particao *p);

#endif // PARTICAO_H

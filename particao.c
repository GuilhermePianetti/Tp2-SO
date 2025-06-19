#include "particao.h"

// Função para encontrar um i-node livre
int encontrarInodeLivre(particao *p) {
    for (int i = 0; i < p->numInodes; i++) {
        if (p->bitmapInodes[i] == 0) {
            return i;
        }
    }
    return -1; // Nenhum i-node livre
}

// Função para encontrar um bloco livre
int encontrarBlocoLivre(particao *p) {
    for (int i = 0; i < p->numBlocos; i++) {
        if (p->bitmap[i] == 0) {
            return i;
        }
    }
    return -1; // Nenhum bloco livre
}

// Função para criar o diretório raiz
int criarDiretorioRaiz(particao *p) {
    int bloco = encontrarBlocoLivre(p);
    if (bloco == -1) {
        printf("Erro: Nenhum bloco livre para diretório raiz.\n");
        return -1;
    }

    // Configurar i-node raiz
    p->inodes[0].numero = 0;
    p->inodes[0].tipo = 1; // Diretório
    p->inodes[0].tamanho = 0; // Inicialmente vazio
    p->inodes[0].data_criacao = time(NULL);
    p->inodes[0].data_modificacao = time(NULL);
    p->inodes[0].data_acesso = time(NULL);
    p->inodes[0].blocos_diretos[0] = bloco;

    // Marcar i-node e bloco como usados
    p->bitmapInodes[0] = 1;
    p->bitmap[bloco] = 1;

    // Inicializar bloco do diretório raiz (vazio inicialmente)
    memset(p->blocos[bloco], 0, p->tamanhoBloco);

    printf("Diretório raiz criado com sucesso (i-node 0).\n");
    return 0;
}


// Função melhorada de inicialização
int inicializarParticao(particao *p, int tamanho, int tamanhoBloco) {
    if (tamanho <= 0 || tamanhoBloco <= 0 || tamanho % tamanhoBloco != 0) {
        printf("Erro: Tamanho inválido da partição ou do bloco.\n");
        return -1;
    }

    // Verificar se o bloco é grande o suficiente para pelo menos uma entrada
    if (tamanhoBloco < sizeof(entrada_diretorio)) {
        printf("Erro: Tamanho do bloco (%d) é menor que uma entrada de diretório (%lu bytes).\n", 
               tamanhoBloco, sizeof(entrada_diretorio));
        return -1;
    }

    p->tamanho = tamanho;
    p->tamanhoBloco = tamanhoBloco;
    p->numBlocos = tamanho / tamanhoBloco;
    p->numInodes = p->numBlocos / 4; // Proporção de 1 i-node para cada 4 blocos

    printf("Debug: Inicializando com:\n");
    printf("  Tamanho entrada_diretorio: %lu bytes\n", sizeof(entrada_diretorio));
    printf("  Entradas por bloco: %d\n", tamanhoBloco / (int)sizeof(entrada_diretorio));

    // Alocar bitmap de blocos
    p->bitmap = (int *)calloc(p->numBlocos, sizeof(int));
    if (!p->bitmap) {
        printf("Erro ao alocar bitmap de blocos.\n");
        return -1;
    }

    // Alocar bitmap de i-nodes
    p->bitmapInodes = (int *)calloc(p->numInodes, sizeof(int));
    if (!p->bitmapInodes) {
        printf("Erro ao alocar bitmap de i-nodes.\n");
        free(p->bitmap);
        return -1;
    }

    // Alocar i-nodes
    p->inodes = (inode *)malloc(p->numInodes * sizeof(inode));
    if (!p->inodes) {
        printf("Erro ao alocar i-nodes.\n");
        free(p->bitmap);
        free(p->bitmapInodes);
        return -1;
    }

    // Inicializar i-nodes
    for (int i = 0; i < p->numInodes; i++) {
        p->inodes[i].numero = i;
        p->inodes[i].tipo = -1; // -1 indica que o i-node está livre
        p->inodes[i].tamanho = 0;
        p->inodes[i].data_criacao = 0;
        p->inodes[i].data_modificacao = 0;
        p->inodes[i].data_acesso = 0;
        for (int j = 0; j < NUM_PONTEIROS_DIRETOS; j++) {
            p->inodes[i].blocos_diretos[j] = -1;
        }
    }

    // Alocar blocos de dados
    p->blocos = (char **)malloc(p->numBlocos * sizeof(char *));
    if (!p->blocos) {
        printf("Erro ao alocar array de blocos.\n");
        free(p->bitmap);
        free(p->bitmapInodes);
        free(p->inodes);
        return -1;
    }

    for (int i = 0; i < p->numBlocos; i++) {
        p->blocos[i] = (char *)calloc(p->tamanhoBloco, sizeof(char));
        if (!p->blocos[i]) {
            printf("Erro ao alocar bloco de dados %d.\n", i);
            // Cleanup de blocos já alocados
            for (int j = 0; j < i; j++) {
                free(p->blocos[j]);
            }
            free(p->blocos);
            free(p->bitmap);
            free(p->bitmapInodes);
            free(p->inodes);
            return -1;
        }
    }

    // Criar diretório raiz
    p->inode_raiz = 0;
    if (criarDiretorioRaiz(p) != 0) {
        printf("Erro ao criar diretório raiz.\n");
        return -1;
    }

    printf("Partição inicializada com sucesso:\n");
    printf("  Tamanho: %d bytes\n", p->tamanho);
    printf("  Tamanho do bloco: %d bytes\n", p->tamanhoBloco);
    printf("  Número de blocos: %d\n", p->numBlocos);
    printf("  Número de i-nodes: %d\n", p->numInodes);
    printf("  Entradas por bloco: %d\n", p->tamanhoBloco / (int)sizeof(entrada_diretorio));
    
    return 0;
}



// Função para listar conteúdo de um diretório
void listarDiretorio(particao *p, int inode_dir) {
    if (p->inodes[inode_dir].tipo != 1) {
        printf("Erro: I-node %d não é um diretório.\n", inode_dir);
        return;
    }

    printf("\nConteúdo do diretório (i-node %d):\n", inode_dir);
    printf("%-20s %-8s %-12s %-20s\n", "Nome", "Tipo", "Tamanho", "Última Modificação");
    printf("------------------------------------------------------------\n");

    int bloco = p->inodes[inode_dir].blocos_diretos[0];
    if (bloco == -1) {
        printf("Diretório vazio.\n");
        return;
    }

    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida) {
            int inode_num = entradas[i].numero_inode;
            char tipo_str[10];
            strcpy(tipo_str, (p->inodes[inode_num].tipo == 1) ? "DIR" : "FILE");
            
            char *time_str = ctime(&p->inodes[inode_num].data_modificacao);
            if (time_str) {
                time_str[strlen(time_str) - 1] = '\0'; // Remove newline
            }
            
            printf("%-20s %-8s %-12d %-20s\n", 
                   entradas[i].nome, 
                   tipo_str,
                   p->inodes[inode_num].tamanho,
                   time_str ? time_str : "N/A");
        }
    }
    
    // Atualizar tempo de acesso
    p->inodes[inode_dir].data_acesso = time(NULL);
}

// Função para mostrar estatísticas da partição
void mostrarEstatisticas(particao *p) {
    int blocos_livres = 0, inodes_livres = 0;
    
    for (int i = 0; i < p->numBlocos; i++) {
        if (p->bitmap[i] == 0) blocos_livres++;
    }
    
    for (int i = 0; i < p->numInodes; i++) {
        if (p->bitmapInodes[i] == 0) inodes_livres++;
    }
    
    printf("\n=== ESTATÍSTICAS DA PARTIÇÃO ===\n");
    printf("Tamanho total: %d bytes\n", p->tamanho);
    printf("Tamanho do bloco: %d bytes\n", p->tamanhoBloco);
    printf("Blocos totais: %d\n", p->numBlocos);
    printf("Blocos livres: %d (%.1f%%)\n", 
           blocos_livres, (float)blocos_livres / p->numBlocos * 100);
    printf("I-nodes totais: %d\n", p->numInodes);
    printf("I-nodes livres: %d (%.1f%%)\n", 
           inodes_livres, (float)inodes_livres / p->numInodes * 100);
    printf("Tamanho da entrada: %lu bytes\n", sizeof(entrada_diretorio));
    printf("Entradas por bloco: %d\n", p->tamanhoBloco / (int)sizeof(entrada_diretorio));
    printf("================================\n\n");
}

// Função para limpar a partição
void liberarParticao(particao *p) {
    if (p) {
        if (p->bitmap) free(p->bitmap);
        if (p->bitmapInodes) free(p->bitmapInodes);
        if (p->inodes) free(p->inodes);
        if (p->blocos) {
            for (int i = 0; i < p->numBlocos; i++) {
                if (p->blocos[i]) free(p->blocos[i]);
            }
            free(p->blocos);
        }
        memset(p, 0, sizeof(particao));
    }
}
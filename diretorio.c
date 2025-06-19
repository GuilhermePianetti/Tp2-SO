#include "diretorio.h"



// Função para buscar uma entrada em um diretório
int buscarEntradaDiretorio(particao *p, int inode_dir, const char *nome) {
    if (p->inodes[inode_dir].tipo != 1) {
        return -1; // Não é um diretório
    }

    int bloco = p->inodes[inode_dir].blocos_diretos[0];
    if (bloco == -1) {
        return -1; // Diretório vazio
    }

    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);
    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida && strcmp(entradas[i].nome, nome) == 0) {
            return entradas[i].numero_inode;
        }
    }

    return -1; // Não encontrado
}

// Função para adicionar entrada em um diretório
int adicionarEntradaDiretorio(particao *p, int inode_dir, const char *nome, int inode_arquivo) {
    if (p->inodes[inode_dir].tipo != 1) {
        return -1; // Não é um diretório
    }

    int bloco = p->inodes[inode_dir].blocos_diretos[0];
    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    // Procurar slot livre
    for (int i = 0; i < max_entradas; i++) {
        if (!entradas[i].valida) {
            strcpy(entradas[i].nome, nome);
            entradas[i].numero_inode = inode_arquivo;
            entradas[i].valida = 1;
            
            // Atualizar tamanho do diretório
            p->inodes[inode_dir].tamanho += sizeof(entrada_diretorio);
            p->inodes[inode_dir].data_modificacao = time(NULL);
            return 0;
        }
    }

    printf("Erro: Diretório cheio (máximo %d entradas por bloco).\n", max_entradas);
    return -1;
}




// Função corrigida para criar diretório
int criarDiretorio(particao *p, const char *nome, int inode_pai) {
    if (!p || !nome || strlen(nome) == 0 || strlen(nome) > MAX_NOME) {
        printf("Erro: Parâmetros inválidos.\n");
        return -1;
    }

    // Verificar se o i-node pai é válido e é um diretório
    if (inode_pai < 0 || inode_pai >= p->numInodes || 
        p->bitmapInodes[inode_pai] == 0 || p->inodes[inode_pai].tipo != 1) {
        printf("Erro: I-node pai inválido ou não é um diretório.\n");
        return -1;
    }

    // Verificar se já existe um arquivo/diretório com esse nome
    if (buscarEntradaDiretorio(p, inode_pai, nome) != -1) {
        printf("Erro: Já existe um arquivo/diretório com o nome '%s'.\n", nome);
        return -1;
    }

    // Encontrar i-node livre
    int inode_novo = encontrarInodeLivre(p);
    if (inode_novo == -1) {
        printf("Erro: Nenhum i-node livre disponível.\n");
        return -1;
    }

    // Encontrar bloco livre
    int bloco_novo = encontrarBlocoLivre(p);
    if (bloco_novo == -1) {
        printf("Erro: Nenhum bloco livre disponível.\n");
        return -1;
    }

    // Configurar novo i-node
    p->inodes[inode_novo].numero = inode_novo;
    p->inodes[inode_novo].tipo = 1; // Diretório
    p->inodes[inode_novo].tamanho = 0;
    p->inodes[inode_novo].data_criacao = time(NULL);
    p->inodes[inode_novo].data_modificacao = time(NULL);
    p->inodes[inode_novo].data_acesso = time(NULL);
    p->inodes[inode_novo].blocos_diretos[0] = bloco_novo;

    // Marcar recursos como usados
    p->bitmapInodes[inode_novo] = 1;
    p->bitmap[bloco_novo] = 1;

    // Inicializar bloco do novo diretório (vazio)
    memset(p->blocos[bloco_novo], 0, p->tamanhoBloco);

    // Adicionar entrada no diretório pai
    if (adicionarEntradaDiretorio(p, inode_pai, nome, inode_novo) != 0) {
        printf("Erro ao adicionar entrada no diretório pai.\n");
        // Rollback
        p->bitmapInodes[inode_novo] = 0;
        p->bitmap[bloco_novo] = 0;
        p->inodes[inode_novo].tipo = -1;
        return -1;
    }

    printf("Diretório '%s' criado com sucesso (i-node %d).\n", nome, inode_novo);
    return inode_novo;
}



int renomearEntrada(particao *p, int inode_dir, const char *nome_antigo, const char *nome_novo) {
    if (!p || !nome_antigo || !nome_novo || strlen(nome_novo) > MAX_NOME) {
        printf("Erro: Parâmetros inválidos para renomear.\n");
        return -1;
    }

    if (p->inodes[inode_dir].tipo != 1) {
        printf("Erro: I-node fornecido não é um diretório.\n");
        return -1;
    }

    int bloco = p->inodes[inode_dir].blocos_diretos[0];
    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida && strcmp(entradas[i].nome, nome_antigo) == 0) {
            // Verificar se já existe o nome novo
            if (buscarEntradaDiretorio(p, inode_dir, nome_novo) != -1) {
                printf("Erro: Já existe uma entrada com o novo nome '%s'.\n", nome_novo);
                return -1;
            }
            strcpy(entradas[i].nome, nome_novo);
            p->inodes[inode_dir].data_modificacao = time(NULL);
            printf("Entrada '%s' renomeada para '%s'.\n", nome_antigo, nome_novo);
            return 0;
        }
    }

    printf("Erro: Entrada '%s' não encontrada para renomear.\n", nome_antigo);
    return -1;
}

int apagarDiretorio(particao *p, int inode_dir_pai, const char *nome) {
    int inode_alvo = buscarEntradaDiretorio(p, inode_dir_pai, nome);

    if (inode_alvo == -1 || p->inodes[inode_alvo].tipo != 1) {
        printf("Erro: Diretório '%s' não encontrado ou não é diretório.\n", nome);
        return -1;
    }

    int bloco_alvo = p->inodes[inode_alvo].blocos_diretos[0];
    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco_alvo];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    // Verificar se o diretório está vazio
    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida) {
            printf("Erro: Diretório '%s' não está vazio.\n", nome);
            return -1;
        }
    }

    // Marcar i-node e bloco como livres
    p->bitmapInodes[inode_alvo] = 0;
    p->bitmap[bloco_alvo] = 0;
    p->inodes[inode_alvo].tipo = -1;

    // Remover entrada do diretório pai
    int bloco_pai = p->inodes[inode_dir_pai].blocos_diretos[0];
    entrada_diretorio *entradas_pai = (entrada_diretorio *)p->blocos[bloco_pai];

    for (int i = 0; i < max_entradas; i++) {
        if (entradas_pai[i].valida && strcmp(entradas_pai[i].nome, nome) == 0) {
            entradas_pai[i].valida = 0;
            p->inodes[inode_dir_pai].tamanho -= sizeof(entrada_diretorio);
            p->inodes[inode_dir_pai].data_modificacao = time(NULL);
            printf("Diretório '%s' removido com sucesso.\n", nome);
            return 0;
        }
    }

    printf("Erro: Entrada '%s' não encontrada no diretório pai.\n", nome);
    return -1;
}
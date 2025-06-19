#include "arquivo.h"

// Função para criar arquivo (básica)
int criarArquivo(particao *p, const char *nome, int inode_pai) {
    if (!p || !nome || strlen(nome) == 0 || strlen(nome) > MAX_NOME) {
        printf("Erro: Parâmetros inválidos.\n");
        return -1;
    }

    // Verificar se o diretório pai é válido
    if (inode_pai < 0 || inode_pai >= p->numInodes || 
        p->bitmapInodes[inode_pai] == 0 || p->inodes[inode_pai].tipo != 1) {
        printf("Erro: Diretório pai inválido.\n");
        return -1;
    }

    // Verificar se já existe
    if (buscarEntradaDiretorio(p, inode_pai, nome) != -1) {
        printf("Erro: Já existe um arquivo com o nome '%s'.\n", nome);
        return -1;
    }

    // Encontrar i-node livre
    int inode_novo = encontrarInodeLivre(p);
    if (inode_novo == -1) {
        printf("Erro: Nenhum i-node livre disponível.\n");
        return -1;
    }

    // Configurar i-node do arquivo
    p->inodes[inode_novo].numero = inode_novo;
    p->inodes[inode_novo].tipo = 0; // Arquivo
    p->inodes[inode_novo].tamanho = 0; // Arquivo vazio inicialmente
    p->inodes[inode_novo].data_criacao = time(NULL);
    p->inodes[inode_novo].data_modificacao = time(NULL);
    p->inodes[inode_novo].data_acesso = time(NULL);

    // Marcar i-node como usado
    p->bitmapInodes[inode_novo] = 1;

    // Adicionar entrada no diretório pai
    if (adicionarEntradaDiretorio(p, inode_pai, nome, inode_novo) != 0) {
        printf("Erro ao adicionar entrada no diretório pai.\n");
        p->bitmapInodes[inode_novo] = 0;
        p->inodes[inode_novo].tipo = -1;
        return -1;
    }

    printf("Arquivo '%s' criado com sucesso (i-node %d).\n", nome, inode_novo);
    return inode_novo;
}

// Função para escrever dados em um arquivo
int escreverArquivo(particao *p, int inode_arquivo, const char *dados, int tamanho) {
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: I-node %d não é um arquivo.\n", inode_arquivo);
        return -1;
    }

    if (tamanho <= 0) {
        printf("Erro: Tamanho de dados inválido.\n");
        return -1;
    }

    // Calcular quantos blocos são necessários
    int blocos_necessarios = (tamanho + p->tamanhoBloco - 1) / p->tamanhoBloco;
    
    if (blocos_necessarios > NUM_PONTEIROS_DIRETOS) {
        printf("Erro: Arquivo muito grande para o sistema atual (máximo %d blocos).\n", NUM_PONTEIROS_DIRETOS);
        return -1;
    }

    // Liberar blocos antigos se o arquivo já tinha dados
    for (int i = 0; i < NUM_PONTEIROS_DIRETOS; i++) {
        if (p->inodes[inode_arquivo].blocos_diretos[i] != -1) {
            p->bitmap[p->inodes[inode_arquivo].blocos_diretos[i]] = 0;
            p->inodes[inode_arquivo].blocos_diretos[i] = -1;
        }
    }

    // Alocar novos blocos
    int blocos_alocados = 0;
    for (int i = 0; i < blocos_necessarios; i++) {
        int bloco = encontrarBlocoLivre(p);
        if (bloco == -1) {
            printf("Erro: Não há blocos livres suficientes.\n");
            // Rollback: liberar blocos já alocados
            for (int j = 0; j < blocos_alocados; j++) {
                p->bitmap[p->inodes[inode_arquivo].blocos_diretos[j]] = 0;
                p->inodes[inode_arquivo].blocos_diretos[j] = -1;
            }
            return -1;
        }
        p->inodes[inode_arquivo].blocos_diretos[i] = bloco;
        p->bitmap[bloco] = 1;
        blocos_alocados++;
    }

    // Escrever dados nos blocos
    int bytes_escritos = 0;
    for (int i = 0; i < blocos_necessarios; i++) {
        int bloco = p->inodes[inode_arquivo].blocos_diretos[i];
        int bytes_para_escrever = (tamanho - bytes_escritos > p->tamanhoBloco) ? 
                                  p->tamanhoBloco : tamanho - bytes_escritos;
        
        memcpy(p->blocos[bloco], dados + bytes_escritos, bytes_para_escrever);
        bytes_escritos += bytes_para_escrever;
    }

    // Atualizar metadados do arquivo
    p->inodes[inode_arquivo].tamanho = tamanho;
    p->inodes[inode_arquivo].data_modificacao = time(NULL);
    p->inodes[inode_arquivo].data_acesso = time(NULL);

    printf("Arquivo escrito com sucesso: %d bytes em %d blocos.\n", tamanho, blocos_necessarios);
    return 0;
}

// Função para importar conteúdo de um arquivo real do sistema
int importarArquivo(particao *p, int inode_arquivo, const char *caminho_arquivo_real) {
    FILE *arquivo_real = fopen(caminho_arquivo_real, "rb");
    if (!arquivo_real) {
        printf("Erro: Não foi possível abrir o arquivo '%s'.\n", caminho_arquivo_real);
        return -1;
    }

    // Obter tamanho do arquivo
    fseek(arquivo_real, 0, SEEK_END);
    long tamanho = ftell(arquivo_real);
    fseek(arquivo_real, 0, SEEK_SET);

    if (tamanho <= 0) {
        printf("Erro: Arquivo vazio ou erro ao ler tamanho.\n");
        fclose(arquivo_real);
        return -1;
    }

    // Limite de tamanho baseado no número de blocos diretos
    long tamanho_maximo = NUM_PONTEIROS_DIRETOS * p->tamanhoBloco;
    if (tamanho > tamanho_maximo) {
        printf("Erro: Arquivo muito grande (%ld bytes). Máximo suportado: %ld bytes.\n", 
               tamanho, tamanho_maximo);
        fclose(arquivo_real);
        return -1;
    }

    // Ler dados do arquivo
    char *buffer = (char *)malloc(tamanho);
    if (!buffer) {
        printf("Erro: Não foi possível alocar memória para o buffer.\n");
        fclose(arquivo_real);
        return -1;
    }

    size_t bytes_lidos = fread(buffer, 1, tamanho, arquivo_real);
    fclose(arquivo_real);

    if (bytes_lidos != tamanho) {
        printf("Erro: Não foi possível ler todo o arquivo.\n");
        free(buffer);
        return -1;
    }

    // Escrever dados no arquivo do sistema de arquivos simulado
    int resultado = escreverArquivo(p, inode_arquivo, buffer, tamanho);
    free(buffer);

    if (resultado == 0) {
        printf("Arquivo '%s' importado com sucesso (%ld bytes).\n", caminho_arquivo_real, tamanho);
    }

    return resultado;
}

// Função para listar conteúdo de um arquivo
void listarConteudoArquivo(particao *p, int inode_arquivo) {
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: I-node %d não é um arquivo.\n", inode_arquivo);
        return;
    }

    int tamanho = p->inodes[inode_arquivo].tamanho;
    if (tamanho == 0) {
        printf("Arquivo vazio.\n");
        return;
    }

    printf("\n=== CONTEÚDO DO ARQUIVO (I-node %d) ===\n", inode_arquivo);
    printf("Tamanho: %d bytes\n", tamanho);
    printf("Conteúdo:\n");
    printf("----------------------------------------\n");

    int bytes_lidos = 0;
    for (int i = 0; i < NUM_PONTEIROS_DIRETOS && bytes_lidos < tamanho; i++) {
        int bloco = p->inodes[inode_arquivo].blocos_diretos[i];
        if (bloco == -1) break;

        int bytes_para_ler = (tamanho - bytes_lidos > p->tamanhoBloco) ? 
                            p->tamanhoBloco : tamanho - bytes_lidos;

        // Imprimir dados do bloco (tratando como texto)
        for (int j = 0; j < bytes_para_ler; j++) {
            char c = p->blocos[bloco][j];
            if (c == '\0') break;
            printf("%c", c);
        }
        bytes_lidos += bytes_para_ler;
    }
    printf("\n----------------------------------------\n");

    // Atualizar tempo de acesso
    p->inodes[inode_arquivo].data_acesso = time(NULL);
}

// Função para renomear arquivo (melhoria da função existente)
int renomearArquivo(particao *p, int inode_dir, const char *nome_antigo, const char *nome_novo) {
    // Verificar se o arquivo existe
    int inode_arquivo = buscarEntradaDiretorio(p, inode_dir, nome_antigo);
    if (inode_arquivo == -1) {
        printf("Erro: Arquivo '%s' não encontrado.\n", nome_antigo);
        return -1;
    }

    // Verificar se é realmente um arquivo
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: '%s' não é um arquivo.\n", nome_antigo);
        return -1;
    }

    // Usar a função de renomear existente
    return renomearEntrada(p, inode_dir, nome_antigo, nome_novo);
}

// Função para mover arquivo
int moverArquivo(particao *p, int inode_dir_origem, const char *nome_arquivo, int inode_dir_destino) {
    // Verificar se o arquivo existe no diretório origem
    int inode_arquivo = buscarEntradaDiretorio(p, inode_dir_origem, nome_arquivo);
    if (inode_arquivo == -1) {
        printf("Erro: Arquivo '%s' não encontrado no diretório origem.\n", nome_arquivo);
        return -1;
    }

    // Verificar se é realmente um arquivo
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: '%s' não é um arquivo.\n", nome_arquivo);
        return -1;
    }

    // Verificar se o diretório destino é válido
    if (p->inodes[inode_dir_destino].tipo != 1) {
        printf("Erro: Diretório destino inválido.\n");
        return -1;
    }

    // Verificar se já existe arquivo com mesmo nome no destino
    if (buscarEntradaDiretorio(p, inode_dir_destino, nome_arquivo) != -1) {
        printf("Erro: Já existe um arquivo com nome '%s' no diretório destino.\n", nome_arquivo);
        return -1;
    }

    // Adicionar entrada no diretório destino
    if (adicionarEntradaDiretorio(p, inode_dir_destino, nome_arquivo, inode_arquivo) != 0) {
        printf("Erro: Não foi possível adicionar arquivo no diretório destino.\n");
        return -1;
    }

    // Remover entrada do diretório origem
    int bloco_origem = p->inodes[inode_dir_origem].blocos_diretos[0];
    entrada_diretorio *entradas_origem = (entrada_diretorio *)p->blocos[bloco_origem];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    for (int i = 0; i < max_entradas; i++) {
        if (entradas_origem[i].valida && strcmp(entradas_origem[i].nome, nome_arquivo) == 0) {
            entradas_origem[i].valida = 0;
            p->inodes[inode_dir_origem].tamanho -= sizeof(entrada_diretorio);
            p->inodes[inode_dir_origem].data_modificacao = time(NULL);
            printf("Arquivo '%s' movido com sucesso.\n", nome_arquivo);
            return 0;
        }
    }

    printf("Erro: Falha ao remover entrada do diretório origem.\n");
    return -1;
}

// Função para apagar arquivo
int apagarArquivo(particao *p, int inode_dir, const char *nome_arquivo) {
    // Verificar se o arquivo existe
    int inode_arquivo = buscarEntradaDiretorio(p, inode_dir, nome_arquivo);
    if (inode_arquivo == -1) {
        printf("Erro: Arquivo '%s' não encontrado.\n", nome_arquivo);
        return -1;
    }

    // Verificar se é realmente um arquivo
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: '%s' não é um arquivo.\n", nome_arquivo);
        return -1;
    }

    // Liberar todos os blocos do arquivo
    for (int i = 0; i < NUM_PONTEIROS_DIRETOS; i++) {
        if (p->inodes[inode_arquivo].blocos_diretos[i] != -1) {
            p->bitmap[p->inodes[inode_arquivo].blocos_diretos[i]] = 0;
            p->inodes[inode_arquivo].blocos_diretos[i] = -1;
        }
    }

    // Marcar i-node como livre
    p->bitmapInodes[inode_arquivo] = 0;
    p->inodes[inode_arquivo].tipo = -1;
    p->inodes[inode_arquivo].tamanho = 0;

    // Remover entrada do diretório
    int bloco_dir = p->inodes[inode_dir].blocos_diretos[0];
    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco_dir];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida && strcmp(entradas[i].nome, nome_arquivo) == 0) {
            entradas[i].valida = 0;
            p->inodes[inode_dir].tamanho -= sizeof(entrada_diretorio);
            p->inodes[inode_dir].data_modificacao = time(NULL);
            printf("Arquivo '%s' apagado com sucesso.\n", nome_arquivo);
            return 0;
        }
    }

    printf("Erro: Falha ao remover entrada do diretório.\n");
    return -1;
}

// Função para buscar arquivo por nome (busca recursiva)
int buscarArquivoRecursivo(particao *p, int inode_dir, const char *nome_arquivo) {
    if (p->inodes[inode_dir].tipo != 1) {
        return -1; // Não é um diretório
    }

    // Buscar no diretório atual
    int resultado = buscarEntradaDiretorio(p, inode_dir, nome_arquivo);
    if (resultado != -1 && p->inodes[resultado].tipo == 0) {
        return resultado; // Encontrou o arquivo
    }

    // Buscar recursivamente em subdiretórios
    int bloco = p->inodes[inode_dir].blocos_diretos[0];
    if (bloco == -1) return -1;

    entrada_diretorio *entradas = (entrada_diretorio *)p->blocos[bloco];
    int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);

    for (int i = 0; i < max_entradas; i++) {
        if (entradas[i].valida && p->inodes[entradas[i].numero_inode].tipo == 1) {
            int resultado_recursivo = buscarArquivoRecursivo(p, entradas[i].numero_inode, nome_arquivo);
            if (resultado_recursivo != -1) {
                return resultado_recursivo;
            }
        }
    }

    return -1; // Não encontrado
}

// Função para exibir informações detalhadas de um arquivo
void informacoesArquivo(particao *p, int inode_arquivo) {
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("Erro: I-node %d não é um arquivo.\n", inode_arquivo);
        return;
    }

    inode *arquivo = &p->inodes[inode_arquivo];
    
    printf("\n=== INFORMAÇÕES DO ARQUIVO ===\n");
    printf("I-node: %d\n", arquivo->numero);
    printf("Tamanho: %d bytes\n", arquivo->tamanho);
    printf("Data de criação: %s", ctime(&arquivo->data_criacao));
    printf("Última modificação: %s", ctime(&arquivo->data_modificacao));
    printf("Último acesso: %s", ctime(&arquivo->data_acesso));
    
    printf("Blocos utilizados: ");
    int blocos_usados = 0;
    for (int i = 0; i < NUM_PONTEIROS_DIRETOS; i++) {
        if (arquivo->blocos_diretos[i] != -1) {
            printf("%d ", arquivo->blocos_diretos[i]);
            blocos_usados++;
        }
    }
    printf("\nNúmero de blocos: %d\n", blocos_usados);
    printf("==============================\n");
}


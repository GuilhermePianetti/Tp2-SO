#include "menu.h"

// Definição das variáveis globais (alocação de memória real)
nivel_caminho caminho_atual[MAX_CAMINHO_DEPTH];
int profundidade_atual = 0;
char caminho_string[MAX_CAMINHO_STR];

// Variáveis globais para o menu
particao sistema;
int diretorio_atual;
bool sistema_inicializado = false;
bool modo_verboso = true;


// Função para limpar o buffer de entrada
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Função para pausar e aguardar entrada do usuário
void pausar() {
    printf("\nPressione Enter para continuar...");
    getchar();
}

// Função para encontrar o nome de um diretório pelo seu i-node
// Busca no diretório pai (se não for a raiz)
char* obterNomeDiretorio(particao* p, int inode_alvo) {
    static char nome_encontrado[MAX_NOME + 1];
    
    // Se é a raiz, retorna "/"
    if (inode_alvo == 0) {
        strcpy(nome_encontrado, "/");
        return nome_encontrado;
    }
    
    // Busca em todos os diretórios para encontrar onde este i-node está referenciado
    for (int dir_inode = 0; dir_inode < p->numInodes; dir_inode++) {
        if (p->bitmapInodes[dir_inode] == 1 && p->inodes[dir_inode].tipo == 1) {
            // Este é um diretório válido, vamos verificar suas entradas
            int bloco_dados = p->inodes[dir_inode].blocos_diretos[0];
            if (bloco_dados != -1) {
                entrada_diretorio* entradas = (entrada_diretorio*)p->blocos[bloco_dados];
                int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);
                
                for (int i = 0; i < max_entradas; i++) {
                    if (entradas[i].numero_inode == inode_alvo && strlen(entradas[i].nome) > 0) {
                        strcpy(nome_encontrado, entradas[i].nome);
                        return nome_encontrado;
                    }
                }
            }
        }
    }
    
    // Se não encontrou, retorna uma identificação pelo i-node
    sprintf(nome_encontrado, "inode_%d", inode_alvo);
    return nome_encontrado;
}



// Função para construir a string do caminho atual
void construirCaminhoString() {
    if (profundidade_atual == 0) {
        strcpy(caminho_string, "/");
        return;
    }
    
    strcpy(caminho_string, "");
    for (int i = 0; i < profundidade_atual; i++) {
        strcat(caminho_string, "/");
        strcat(caminho_string, caminho_atual[i].nome);
    }
}

// Função para exibir o caminho atual
void exibirCaminhoAtual() {
    construirCaminhoString();
    if (profundidade_atual == 0) {
        printf("📁 Localização atual: / (raiz)\n");
    } else {
        printf("📁 Localização atual: %s\n", caminho_string);
    }
}

// Função para navegar para um diretório (atualiza o caminho)
int navegarPara(particao* p, const char* nome_diretorio) {
    if (strcmp(nome_diretorio, "..") == 0) {
        // Voltar um nível
        if (profundidade_atual > 0) {
            profundidade_atual--;
            diretorio_atual = (profundidade_atual == 0) ? 0 : caminho_atual[profundidade_atual - 1].inode;
        } else {
            diretorio_atual = 0; // Já está na raiz
        }
        return 0; // Sucesso
    }
    
    // Buscar o diretório no diretório atual
    int inode_destino = buscarEntradaDiretorio(p, diretorio_atual, nome_diretorio);
    
    if (inode_destino != -1 && p->inodes[inode_destino].tipo == 1) {
        // Adicionar ao caminho
        if (profundidade_atual < MAX_CAMINHO_DEPTH - 1) {
            caminho_atual[profundidade_atual].inode = inode_destino;
            strcpy(caminho_atual[profundidade_atual].nome, nome_diretorio);
            profundidade_atual++;
            diretorio_atual = inode_destino;
            return 0; // Sucesso
        } else {
            return -2; // Caminho muito profundo
        }
    }
    
    return -1; // Diretório não encontrado
}

// Função para inicializar o sistema de caminho
void inicializarSistemaCaminho() {
    profundidade_atual = 0;
    diretorio_atual = 0;
    memset(caminho_atual, 0, sizeof(caminho_atual));
    strcpy(caminho_string, "/");
}

// Função para navegar para a raiz
void irParaRaiz() {
    profundidade_atual = 0;
    diretorio_atual = 0;
}

// Função para obter o caminho atual como string
const char* obterCaminhoAtual() {
    construirCaminhoString();
    return caminho_string;
}

void menuNavegacao() {
    char nome[MAX_NOME + 1];
    
    printf("\n🔄 Navegar para diretório\n");
    printf("Digite:\n");
    printf("  • Nome do diretório para entrar\n");
    printf("  • '..' para voltar um nível\n");
    printf("  • '/' para ir à raiz\n");
    printf("Opção: ");
    
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;
    
    if (strlen(nome) == 0) {
        printf("❌ Nome inválido.\n");
        return;
    }
    
    if (strcmp(nome, "/") == 0) {
        irParaRaiz();
        printf("✅ Voltou para o diretório raiz.\n");
        return;
    }
    
    int resultado = navegarPara(&sistema, nome);
    
    switch (resultado) {
        case 0:
            if (strcmp(nome, "..") == 0) {
                printf("✅ Voltou um nível no diretório.\n");
            } else {
                printf("✅ Navegou para o diretório '%s'.\n", nome);
            }
            break;
        case -1:
            printf("❌ Diretório '%s' não encontrado.\n", nome);
            break;
        case -2:
            printf("❌ Caminho muito profundo (máximo %d níveis).\n", MAX_CAMINHO_DEPTH);
            break;
        default:
            printf("❌ Erro desconhecido na navegação.\n");
    }
}

// Função para mostrar navegação com breadcrumb
void exibirCaminhoComBreadcrumb() {
    construirCaminhoString();
    
    printf("📁 Localização: %s", caminho_string);
    
    if (profundidade_atual > 0) {
        printf(" (nível %d)", profundidade_atual);
    }
    
    printf("\n");
    
    // Mostrar breadcrumb navegável (opcional)
    if (profundidade_atual > 1) {
        printf("💡 Dica: Use '..' para voltar ou '/' para ir à raiz\n");
    }
}

// Função para resolver um caminho completo e retornar o i-node correspondente
// Retorna -1 se o caminho não for válido ou não existir
int resolverCaminhoCompleto(particao* p, const char* caminho) {
    if (!caminho || strlen(caminho) == 0) {
        return -1;
    }
    
    // Se o caminho é apenas "/", retorna a raiz
    if (strcmp(caminho, "/") == 0) {
        return 0;
    }
    
    // Se não começa com "/", é um caminho relativo - não suportado por enquanto
    if (caminho[0] != '/') {
        return -1;
    }
    
    // Copia o caminho para manipulação
    char caminho_trabalho[256];
    strcpy(caminho_trabalho, caminho);
    
    // Remove '/' no final se existir (exceto se for apenas "/")
    int len = strlen(caminho_trabalho);
    if (len > 1 && caminho_trabalho[len - 1] == '/') {
        caminho_trabalho[len - 1] = '\0';
    }
    
    int inode_atual = 0; // Começar na raiz
    
    // Dividir o caminho em componentes
    char* token = strtok(caminho_trabalho + 1, "/"); // +1 para pular o primeiro "/"
    
    while (token != NULL) {
        // Buscar o próximo componente no diretório atual
        int proximo_inode = buscarEntradaDiretorio(p, inode_atual, token);
        
        if (proximo_inode == -1) {
            // Componente não encontrado
            return -1;
        }
        
        // Verificar se é um diretório (necessário para continuar navegando)
        if (p->inodes[proximo_inode].tipo != 1) {
            // Não é um diretório, mas ainda há mais componentes no caminho
            if (strtok(NULL, "/") != NULL) {
                return -1; // Erro: tentando navegar através de um arquivo
            }
            // É o último componente e é um arquivo - isso é válido
            return proximo_inode;
        }
        
        inode_atual = proximo_inode;
        token = strtok(NULL, "/");
    }
    
    return inode_atual;
}

// Função para verificar se um caminho é válido e se refere a um diretório
bool verificarCaminhoEDiretorio(particao* p, const char* caminho) {
    int inode = resolverCaminhoCompleto(p, caminho);
    if (inode == -1) {
        return false;
    }
    return p->inodes[inode].tipo == 1; // 1 = diretório
}

// Função para obter sugestões de caminhos (para autocompletar ou ajudar o usuário)
void mostrarSugestoesCaminhos(particao* p, int inode_base, const char* prefixo_caminho) {
    printf("💡 Diretórios disponíveis:\n");
    
    // Mostrar conteúdo do diretório base
    int bloco_dados = p->inodes[inode_base].blocos_diretos[0];
    if (bloco_dados != -1) {
        entrada_diretorio* entradas = (entrada_diretorio*)p->blocos[bloco_dados];
        int max_entradas = p->tamanhoBloco / sizeof(entrada_diretorio);
        
        for (int i = 0; i < max_entradas; i++) {
            if (entradas[i].numero_inode != -1 && strlen(entradas[i].nome) > 0) {
                int inode_entrada = entradas[i].numero_inode;
                if (p->inodes[inode_entrada].tipo == 1) { // Apenas diretórios
                    printf("   %s%s/\n", prefixo_caminho, entradas[i].nome);
                }
            }
        }
    }
}

// Versão melhorada da função de mover arquivo que aceita caminhos
int moverArquivoComCaminho(particao* p, int inode_dir_origem, const char* nome_arquivo, const char* caminho_destino) {
    // Primeiro, verificar se o arquivo existe no diretório de origem
    int inode_arquivo = buscarEntradaDiretorio(p, inode_dir_origem, nome_arquivo);
    if (inode_arquivo == -1) {
        printf("❌ Arquivo '%s' não encontrado no diretório atual.\n", nome_arquivo);
        return -1;
    }
    
    // Verificar se é realmente um arquivo
    if (p->inodes[inode_arquivo].tipo != 0) {
        printf("❌ '%s' não é um arquivo.\n", nome_arquivo);
        return -1;
    }
    
    // Resolver o caminho de destino
    int inode_destino = resolverCaminhoCompleto(p, caminho_destino);
    if (inode_destino == -1) {
        printf("❌ Caminho de destino '%s' não encontrado.\n", caminho_destino);
        printf("💡 Certifique-se de que o caminho existe e está correto.\n");
        printf("   Exemplo: /diretorio1/subdiretorio\n");
        return -1;
    }
    
    // Verificar se o destino é um diretório
    if (p->inodes[inode_destino].tipo != 1) {
        printf("❌ O destino '%s' não é um diretório.\n", caminho_destino);
        return -1;
    }
    
    // Verificar se já existe um arquivo com o mesmo nome no destino
    if (buscarEntradaDiretorio(p, inode_destino, nome_arquivo) != -1) {
        printf("❌ Já existe um arquivo com o nome '%s' no destino.\n", nome_arquivo);
        return -1;
    }
    
    // Usar a função original de mover arquivo
    int resultado = moverArquivo(p, inode_dir_origem, nome_arquivo, inode_destino);
    
    if (resultado == 0) {
        printf("✅ Arquivo '%s' movido para '%s' com sucesso!\n", nome_arquivo, caminho_destino);
    }
    
    return resultado;
}

// Função para navegar usando caminho completo (atualiza o estado global)
int navegarParaCaminhoCompleto(particao* p, const char* caminho) {
    if (!caminho || strlen(caminho) == 0) {
        return -1;
    }
    
    // Resolver o caminho
    int inode_destino = resolverCaminhoCompleto(p, caminho);
    if (inode_destino == -1) {
        return -1; // Caminho não encontrado
    }
    
    // Verificar se é um diretório
    if (p->inodes[inode_destino].tipo != 1) {
        return -2; // Não é um diretório
    }
    
    // Atualizar o estado global de navegação
    if (strcmp(caminho, "/") == 0) {
        // Ir para a raiz
        irParaRaiz();
        return 0;
    }
    
    // Reconstruir o caminho atual baseado no caminho fornecido
    char caminho_trabalho[256];
    strcpy(caminho_trabalho, caminho);
    
    // Remove '/' no final se existir
    int len = strlen(caminho_trabalho);
    if (len > 1 && caminho_trabalho[len - 1] == '/') {
        caminho_trabalho[len - 1] = '\0';
    }
    
    // Resetar o caminho atual
    profundidade_atual = 0;
    diretorio_atual = 0;
    
    // Navegar componente por componente
    char* token = strtok(caminho_trabalho + 1, "/");
    
    while (token != NULL) {
        int resultado = navegarPara(p, token);
        if (resultado != 0) {
            return resultado;
        }
        token = strtok(NULL, "/");
    }
    
    return 0;
}

// Versão atualizada do case 7 no menu de arquivos
void menuMoverArquivoAtualizado() {
    char nome[MAX_NOME + 1], caminho_destino[256];
    int opcao_entrada;
    
    printf("\n🔄 Mover arquivo\n");
    printf("Nome do arquivo: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;
    
    printf("\nEscolha como especificar o destino:\n");
    printf("1. 📁 Caminho completo (ex: /docs/imagens)\n");
    printf("2. 🆔 I-node do diretório (método antigo)\n");
    printf("Opção: ");
    
    if (scanf("%d", &opcao_entrada) != 1) {
        printf("❌ Opção inválida!\n");
        limparBuffer();
        return;
    }
    limparBuffer();
    
    if (opcao_entrada == 1) {
        // Usar caminho completo
        printf("Caminho do diretório destino: ");
        fgets(caminho_destino, sizeof(caminho_destino), stdin);
        caminho_destino[strcspn(caminho_destino, "\n")] = 0;
        
        // Mostrar alguns diretórios disponíveis como sugestão
        printf("\n💡 Alguns diretórios disponíveis a partir da raiz:\n");
        mostrarSugestoesCaminhos(&sistema, 0, "/");
        
        if (modo_verboso) {
            printf("⏳ Movendo arquivo '%s' para '%s'...\n", nome, caminho_destino);
        }
        
        moverArquivoComCaminho(&sistema, diretorio_atual, nome, caminho_destino);
        
    } else if (opcao_entrada == 2) {
        // Usar i-node (método antigo)
        int inode_destino;
        printf("I-node do diretório destino (0 = raiz): ");
        if (scanf("%d", &inode_destino) != 1) {
            printf("❌ I-node inválido!\n");
            limparBuffer();
            return;
        }
        limparBuffer();
        
        if (modo_verboso) {
            printf("⏳ Movendo arquivo '%s'...\n", nome);
        }
        
        if (moverArquivo(&sistema, diretorio_atual, nome, inode_destino) == 0) {
            printf("✅ Arquivo movido com sucesso!\n");
        } else {
            printf("❌ Falha ao mover arquivo.\n");
        }
    } else {
        printf("❌ Opção inválida!\n");
    }
}

// Função adicional para navegação avançada no menu
void menuNavegacaoAvancada() {
    char entrada[256];
    int opcao;
    
    printf("\n🔄 Navegação Avançada\n");
    printf("1. 📁 Navegar usando caminho completo\n");
    printf("2. 📋 Navegar usando nome do diretório (atual)\n");
    printf("Opção: ");
    
    if (scanf("%d", &opcao) != 1) {
        printf("❌ Opção inválida!\n");
        limparBuffer();
        return;
    }
    limparBuffer();
    
    if (opcao == 1) {
        printf("Digite o caminho completo (ex: /docs/imagens): ");
        fgets(entrada, sizeof(entrada), stdin);
        entrada[strcspn(entrada, "\n")] = 0;
        
        if (strlen(entrada) == 0) {
            printf("❌ Caminho inválido.\n");
            return;
        }
        
        int resultado = navegarParaCaminhoCompleto(&sistema, entrada);
        
        switch (resultado) {
            case 0:
                printf("✅ Navegou para '%s' com sucesso.\n", entrada);
                break;
            case -1:
                printf("❌ Caminho '%s' não encontrado.\n", entrada);
                break;
            case -2:
                printf("❌ '%s' não é um diretório.\n", entrada);
                break;
            default:
                printf("❌ Erro desconhecido na navegação.\n");
        }
    } else if (opcao == 2) {
        // Usar a função de navegação original
        menuNavegacao();
    } else {
        printf("❌ Opção inválida!\n");
    }
}

// Função para inicializar o sistema
void menuInicializarSistema() {
    int tamanho, tamanhoBloco;
    
    printf("\n═══════════════════════════════════════════\n");
    printf("🚀 INICIALIZAÇÃO DO SISTEMA DE ARQUIVOS\n");
    printf("═══════════════════════════════════════════\n");
    printf("📋 Configurações recomendadas:\n");
    printf("   • Tamanho da partição: 8192-65536 bytes\n");
    printf("   • Tamanho do bloco: 512-1024 bytes\n\n");
    
    do {
        printf("💾 Digite o tamanho da partição (em bytes): ");
        if (scanf("%d", &tamanho) != 1) {
            printf("❌ Erro: Digite um número válido.\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        if (tamanho < 1024 || tamanho > 1000000) {
            printf("⚠️  Aviso: Tamanho fora da faixa recomendada.\n");
        }
        break;
    } while (1);
    
    do {
        printf("🔧 Digite o tamanho do bloco (em bytes): ");
        if (scanf("%d", &tamanhoBloco) != 1) {
            printf("❌ Erro: Digite um número válido.\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        if (tamanho % tamanhoBloco != 0) {
            printf("❌ Erro: O tamanho da partição deve ser múltiplo do tamanho do bloco.\n");
            continue;
        }
        break;
    } while (1);
    
    printf("\n⏳ Inicializando sistema de arquivos...\n");
    
    if (inicializarParticao(&sistema, tamanho, tamanhoBloco) == 0) {
        sistema_inicializado = true;
        inicializarSistemaCaminho();
        diretorio_atual = 0; // Começar na raiz
        printf("✅ Sistema inicializado com sucesso!\n");
        mostrarEstatisticas(&sistema);
    } else {
        printf("❌ Falha na inicialização do sistema.\n");
    }
    
    pausar();
}

// Menu para operações com diretórios
void menuDiretorios() {
    int opcao;
    char nome[MAX_NOME + 1], nome_novo[MAX_NOME + 1];
    
    do {
        printf("\n═══════════════════════════════════════════\n");
        printf("📁 OPERAÇÕES COM DIRETÓRIOS\n");
        printf("═══════════════════════════════════════════\n");
        exibirCaminhoAtual();
        printf("\n1. 📋 Listar conteúdo do diretório atual\n");
        printf("2. ➕ Criar novo diretório\n");
        printf("3. ✏️  Renomear diretório\n");
        printf("4. 🗑️  Apagar diretório\n");
        printf("5. 🔄 Navegar para diretório\n");
        printf("0. ⬅️  Voltar ao menu principal\n");
        printf("═══════════════════════════════════════════\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("❌ Opção inválida!\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        switch (opcao) {
            case 1:
                printf("\n📋 Conteúdo do diretório:\n");
                listarDiretorio(&sistema, diretorio_atual);
                pausar();
                break;
                
            case 2:
                printf("\n➕ Criar novo diretório\n");
                printf("Digite o nome do diretório: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0; // Remove newline
                
                if (modo_verboso) {
                    printf("⏳ Criando diretório '%s'...\n", nome);
                }
                
                if (criarDiretorio(&sistema, nome, diretorio_atual) != -1) {
                    printf("✅ Diretório criado com sucesso!\n");
                } else {
                    printf("❌ Falha ao criar diretório.\n");
                }
                pausar();
                break;
                
            case 3:
                printf("\n✏️  Renomear diretório\n");
                printf("Nome atual do diretório: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("Novo nome: ");
                fgets(nome_novo, sizeof(nome_novo), stdin);
                nome_novo[strcspn(nome_novo, "\n")] = 0;
                
                if (modo_verboso) {
                    printf("⏳ Renomeando '%s' para '%s'...\n", nome, nome_novo);
                }
                
                if (renomearEntrada(&sistema, diretorio_atual, nome, nome_novo) == 0) {
                    printf("✅ Diretório renomeado com sucesso!\n");
                } else {
                    printf("❌ Falha ao renomear diretório.\n");
                }
                pausar();
                break;
                
            case 4:
                printf("\n🗑️  Apagar diretório\n");
                printf("⚠️  ATENÇÃO: Esta operação é irreversível!\n");
                printf("Nome do diretório a apagar: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("Tem certeza? (s/N): ");
                char confirmacao;
                scanf("%c", &confirmacao);
                limparBuffer();
                
                if (confirmacao == 's' || confirmacao == 'S') {
                    if (modo_verboso) {
                        printf("⏳ Apagando diretório '%s'...\n", nome);
                    }
                    
                    if (apagarDiretorio(&sistema, diretorio_atual, nome) == 0) {
                        printf("✅ Diretório apagado com sucesso!\n");
                    } else {
                        printf("❌ Falha ao apagar diretório.\n");
                    }
                } else {
                    printf("❌ Operação cancelada.\n");
                }
                pausar();
                break;
                
            case 5:
                menuNavegacaoAvancada();
                pausar();
                break;
                
            case 0:
                break;
                
            default:
                printf("❌ Opção inválida!\n");
                pausar();
        }
    } while (opcao != 0);
}

// Menu para operações com arquivos
void menuArquivos() {
    int opcao;
    char nome[MAX_NOME + 1], nome_novo[MAX_NOME + 1], caminho[256];
    
    do {
        printf("\n═══════════════════════════════════════════\n");
        printf("📄 OPERAÇÕES COM ARQUIVOS\n");
        printf("═══════════════════════════════════════════\n");
        exibirCaminhoAtual();
        printf("\n1. 📋 Listar arquivos do diretório atual\n");
        printf("2. ➕ Criar arquivo vazio\n");
        printf("3. 📥 Importar arquivo do sistema\n");
        printf("4. 👁️  Visualizar conteúdo do arquivo\n");
        printf("5. ℹ️  Informações detalhadas do arquivo\n");
        printf("6. ✏️  Renomear arquivo\n");
        printf("7. 🔄 Mover arquivo\n");
        printf("8. 🗑️  Apagar arquivo\n");
        printf("9. 🔍 Buscar arquivo (recursivo)\n");
        printf("0. ⬅️  Voltar ao menu principal\n");
        printf("═══════════════════════════════════════════\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("❌ Opção inválida!\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        switch (opcao) {
            case 1:
                printf("\n📋 Arquivos no diretório atual:\n");
                listarDiretorio(&sistema, diretorio_atual);
                pausar();
                break;
                
            case 2:
                printf("\n➕ Criar arquivo vazio\n");
                printf("Nome do arquivo: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                if (modo_verboso) {
                    printf("⏳ Criando arquivo '%s'...\n", nome);
                }
                
                if (criarArquivo(&sistema, nome, diretorio_atual) != -1) {
                    printf("✅ Arquivo criado com sucesso!\n");
                } else {
                    printf("❌ Falha ao criar arquivo.\n");
                }
                pausar();
                break;
                
            case 3:
                printf("\n📥 Importar arquivo do sistema\n");
                printf("Nome do arquivo no simulador: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("Caminho do arquivo real: ");
                fgets(caminho, sizeof(caminho), stdin);
                caminho[strcspn(caminho, "\n")] = 0;
                
                // Criar arquivo primeiro
                int inode_arquivo = criarArquivo(&sistema, nome, diretorio_atual);
                if (inode_arquivo != -1) {
                    if (modo_verboso) {
                        printf("⏳ Importando conteúdo de '%s'...\n", caminho);
                    }
                    
                    if (importarArquivo(&sistema, inode_arquivo, caminho) == 0) {
                        printf("✅ Arquivo importado com sucesso!\n");
                    } else {
                        printf("❌ Falha ao importar conteúdo do arquivo.\n");
                    }
                } else {
                    printf("❌ Falha ao criar arquivo no simulador.\n");
                }
                pausar();
                break;
                
            case 4:
                printf("\n👁️  Visualizar conteúdo do arquivo\n");
                printf("Nome do arquivo: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                int inode_vis = buscarEntradaDiretorio(&sistema, diretorio_atual, nome);
                if (inode_vis != -1 && sistema.inodes[inode_vis].tipo == 0) {
                    listarConteudoArquivo(&sistema, inode_vis);
                } else {
                    printf("❌ Arquivo não encontrado.\n");
                }
                pausar();
                break;
                
            case 5:
                printf("\nℹ️  Informações detalhadas do arquivo\n");
                printf("Nome do arquivo: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                int inode_info = buscarEntradaDiretorio(&sistema, diretorio_atual, nome);
                if (inode_info != -1 && sistema.inodes[inode_info].tipo == 0) {
                    informacoesArquivo(&sistema, inode_info);
                } else {
                    printf("❌ Arquivo não encontrado.\n");
                }
                pausar();
                break;
                
            case 6:
                printf("\n✏️  Renomear arquivo\n");
                printf("Nome atual: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("Novo nome: ");
                fgets(nome_novo, sizeof(nome_novo), stdin);
                nome_novo[strcspn(nome_novo, "\n")] = 0;
                
                if (modo_verboso) {
                    printf("⏳ Renomeando '%s' para '%s'...\n", nome, nome_novo);
                }
                
                if (renomearArquivo(&sistema, diretorio_atual, nome, nome_novo) == 0) {
                    printf("✅ Arquivo renomeado com sucesso!\n");
                } else {
                    printf("❌ Falha ao renomear arquivo.\n");
                }
                pausar();
                break;
                
            case 7:
                menuMoverArquivoAtualizado();
                pausar();
                break;
                            
            case 8:
                printf("\n🗑️  Apagar arquivo\n");
                printf("⚠️  ATENÇÃO: Esta operação é irreversível!\n");
                printf("Nome do arquivo: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("Tem certeza? (s/N): ");
                char conf;
                scanf("%c", &conf);
                limparBuffer();
                
                if (conf == 's' || conf == 'S') {
                    if (modo_verboso) {
                        printf("⏳ Apagando arquivo '%s'...\n", nome);
                    }
                    
                    if (apagarArquivo(&sistema, diretorio_atual, nome) == 0) {
                        printf("✅ Arquivo apagado com sucesso!\n");
                    } else {
                        printf("❌ Falha ao apagar arquivo.\n");
                    }
                } else {
                    printf("❌ Operação cancelada.\n");
                }
                pausar();
                break;
                
            case 9:
                printf("\n🔍 Buscar arquivo (recursivo)\n");
                printf("Nome do arquivo: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                if (modo_verboso) {
                    printf("⏳ Buscando arquivo '%s'...\n", nome);
                }
                
                int resultado = buscarArquivoRecursivo(&sistema, 0, nome); // Buscar a partir da raiz
                if (resultado != -1) {
                    printf("✅ Arquivo encontrado (i-node %d)!\n", resultado);
                    informacoesArquivo(&sistema, resultado);
                } else {
                    printf("❌ Arquivo não encontrado.\n");
                }
                pausar();
                break;
                
            case 0:
                break;
                
            default:
                printf("❌ Opção inválida!\n");
                pausar();
        }
    } while (opcao != 0);
}

// Menu de configurações
void menuConfiguracoes() {
    int opcao;
    
    do {
        printf("\n═══════════════════════════════════════════\n");
        printf("⚙️  CONFIGURAÇÕES E INFORMAÇÕES\n");
        printf("═══════════════════════════════════════════\n");
        printf("1. 📊 Mostrar estatísticas do sistema\n");
        printf("2. 🔧 Alternar modo verboso (atual: %s)\n", modo_verboso ? "ATIVO" : "INATIVO");
        printf("3. 🆔 Mostrar informações dos i-nodes\n");
        printf("4. 🗺️  Mostrar mapa de bits\n");
        printf("5. 🔄 Reinicializar sistema\n");
        printf("0. ⬅️  Voltar ao menu principal\n");
        printf("═══════════════════════════════════════════\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("❌ Opção inválida!\n");
            limparBuffer();
            continue;
        }
        limparBuffer();
        
        switch (opcao) {
            case 1:
                mostrarEstatisticas(&sistema);
                pausar();
                break;
                
            case 2:
                modo_verboso = !modo_verboso;
                printf("✅ Modo verboso %s!\n", modo_verboso ? "ATIVADO" : "DESATIVADO");
                pausar();
                break;
                
            case 3:
                printf("\n🆔 Informações dos I-nodes:\n");
                printf("%-6s %-8s %-10s %-20s\n", "I-node", "Tipo", "Tamanho", "Última Modificação");
                printf("─────────────────────────────────────────────────\n");
                
                for (int i = 0; i < sistema.numInodes; i++) {
                    if (sistema.bitmapInodes[i] == 1) {
                        char tipo_str[10];
                        if (sistema.inodes[i].tipo == 1) strcpy(tipo_str, "DIR");
                        else if (sistema.inodes[i].tipo == 0) strcpy(tipo_str, "FILE");
                        else strcpy(tipo_str, "LIVRE");
                        
                        char *time_str = ctime(&sistema.inodes[i].data_modificacao);
                        if (time_str) time_str[strlen(time_str) - 1] = '\0';
                        
                        printf("%-6d %-8s %-10d %-20s\n", 
                               i, tipo_str, sistema.inodes[i].tamanho, 
                               time_str ? time_str : "N/A");
                    }
                }
                pausar();
                break;
                
            case 4:
                printf("\n🗺️  Mapa de bits dos blocos (L=Livre, U=Usado):\n");
                for (int i = 0; i < sistema.numBlocos; i++) {
                    if (i % 32 == 0) printf("\nBlocos %3d-%3d: ", i, i + 31);
                    printf("%c", sistema.bitmap[i] ? 'U' : 'L');
                }
                printf("\n\n🗺️  Mapa de bits dos i-nodes (L=Livre, U=Usado):\n");
                for (int i = 0; i < sistema.numInodes; i++) {
                    if (i % 32 == 0) printf("\nI-nodes %2d-%2d: ", i, i + 31);
                    printf("%c", sistema.bitmapInodes[i] ? 'U' : 'L');
                }
                printf("\n");
                pausar();
                break;
                
            case 5:
                printf("\n🔄 Reinicializar sistema\n");
                printf("⚠️  ATENÇÃO: Todos os dados serão perdidos!\n");
                printf("Tem certeza? (s/N): ");
                char conf;
                scanf("%c", &conf);
                limparBuffer();
                
                if (conf == 's' || conf == 'S') {
                    liberarParticao(&sistema);
                    sistema_inicializado = false;
                    diretorio_atual = 0;
                    printf("✅ Sistema reinicializado. Execute a inicialização novamente.\n");
                } else {
                    printf("❌ Operação cancelada.\n");
                }
                pausar();
                break;
                
            case 0:
                break;
                
            default:
                printf("❌ Opção inválida!\n");
                pausar();
        }
    } while (opcao != 0);
}

// Menu principal
void menuPrincipal() {
    int opcao;
    
    do {
        system("clear || cls"); // Limpar tela (Linux/Mac || Windows)
        
        printf("╔═══════════════════════════════════════════╗\n");
        printf("║        🗄️  SIMULADOR DE SISTEMA DE        ║\n");
        printf("║              ARQUIVOS v1.0               ║\n");
        printf("╚═══════════════════════════════════════════╝\n\n");
        
        if (sistema_inicializado) {
            printf("🟢 Sistema INICIALIZADO\n");
            exibirCaminhoAtual();
        } else {
            printf("🔴 Sistema NÃO INICIALIZADO\n");
            printf("📁 Execute a inicialização primeiro!\n");
        }
        
        printf("\n═══════════════════════════════════════════\n");
        printf("📋 MENU PRINCIPAL\n");
        printf("═══════════════════════════════════════════\n");
        
        if (!sistema_inicializado) {
            printf("1. 🚀 Inicializar sistema de arquivos\n");
            printf("9. ❓ Sobre o simulador\n");
            printf("0. 🚪 Sair\n");
        } else {
            printf("1. 🚀 Reinicializar sistema\n");
            printf("2. 📁 Operações com diretórios\n");
            printf("3. 📄 Operações com arquivos\n");
            printf("4. ⚙️  Configurações e informações\n");
            printf("5. 🤖 Execução automática de comandos\n");
            printf("9. ❓ Sobre o simulador\n");
            printf("0. 🚪 Sair\n");
        }
        
        printf("═══════════════════════════════════════════\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("❌ Opção inválida!\n");
            limparBuffer();
            pausar();
            continue;
        }
        limparBuffer();
        
        switch (opcao) {
            case 1:
                if (!sistema_inicializado) {
                    menuInicializarSistema();
                } else {
                    liberarParticao(&sistema);
                    sistema_inicializado = false;
                    menuInicializarSistema();
                }
                break;
                
            case 2:
                if (sistema_inicializado) {
                    menuDiretorios();
                } else {
                    printf("❌ Sistema não inicializado!\n");
                    pausar();
                }
                break;
                
            case 3:
                if (sistema_inicializado) {
                    menuArquivos();
                } else {
                    printf("❌ Sistema não inicializado!\n");
                    pausar();
                }
                break;
                
            case 4:
                if (sistema_inicializado) {
                    menuConfiguracoes();
                } else {
                    printf("❌ Sistema não inicializado!\n");
                    pausar();
                }
                break;
            case 5:
                if (sistema_inicializado) {
                    menuExecucaoAutomatica();
                } else {
                    printf("❌ Sistema não inicializado!\n");
                    pausar();
                }
                break;
                
            case 9:
                printf("\n╔═══════════════════════════════════════════╗\n");
                printf("║           📖 SOBRE O SIMULADOR            ║\n");
                printf("╚═══════════════════════════════════════════╝\n");
                printf("🔹 Simulador de Sistema de Arquivos v1.0\n");
                printf("🔹 Implementa conceitos de i-nodes e blocos\n");
                printf("🔹 Suporte a diretórios e arquivos\n");
                printf("🔹 Operações: criar, listar, renomear, mover, apagar\n");
                printf("🔹 Importação de arquivos reais do sistema\n");
                printf("🔹 Busca recursiva e estatísticas detalhadas\n\n");
                printf("📋 Estruturas principais:\n");
                printf("   • I-nodes: metadados dos arquivos/diretórios\n");
                printf("   • Blocos: armazenamento de dados\n");
                printf("   • Bitmaps: controle de recursos livres/usados\n");
                printf("   • Entradas de diretório: mapeamento nome→i-node\n\n");
                printf("⚡ Desenvolvido para fins educacionais\n");
                pausar();
                break;
                
            case 0:
                if (sistema_inicializado) {
                    printf("\n🔄 Liberando recursos...\n");
                    liberarParticao(&sistema);
                }
                printf("👋 Obrigado por usar o simulador!\n");
                break;
                
            default:
                printf("❌ Opção inválida!\n");
                pausar();
        }
    } while (opcao != 0);
}
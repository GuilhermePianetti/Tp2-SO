#include "modoAutomatico.h"

bool modo_automatico = false;
FILE* arquivo_comandos = NULL;
int linha_atual = 0;
bool pausar_em_erro = true;
bool mostrar_comandos = true;
extern particao sistema;
extern int diretorio_atual;
extern char caminho_string[1024]; // ou o tamanho real


// ========================================
// FUNÇÕES DE PARSING E EXECUÇÃO
// ========================================

// Função para remover espaços em branco no início e fim de uma string
void trim(char* str) {
    // Remove espaços do início
    int inicio = 0;
    while (str[inicio] == ' ' || str[inicio] == '\t') {
        inicio++;
    }
    
    // Move a string para o início
    int i = 0;
    while (str[inicio + i] != '\0') {
        str[i] = str[inicio + i];
        i++;
    }
    str[i] = '\0';
    
    // Remove espaços do fim
    int fim = strlen(str) - 1;
    while (fim >= 0 && (str[fim] == ' ' || str[fim] == '\t' || str[fim] == '\n' || str[fim] == '\r')) {
        str[fim] = '\0';
        fim--;
    }
}

// Função para parsear uma linha de comando
int parsearComando(const char* linha, comando_automatico* cmd) {
    if (!linha || !cmd || strlen(linha) == 0) {
        return -1;
    }
    
    // Copia a linha para manipulação
    char linha_trabalho[MAX_LINHA_COMANDO];
    strcpy(linha_trabalho, linha);
    trim(linha_trabalho);
    
    // Ignora linhas vazias e comentários
    if (strlen(linha_trabalho) == 0 || linha_trabalho[0] == '#') {
        return 0; // Linha válida mas sem comando
    }
    
    // Inicializa a estrutura
    memset(cmd, 0, sizeof(comando_automatico));
    
    // Parseia o comando e argumentos
    char* token = strtok(linha_trabalho, " \t");
    if (token) {
        strcpy(cmd->comando, token);
        
        // Parseia os argumentos
        while ((token = strtok(NULL, " \t")) != NULL && cmd->num_args < MAX_ARGS) {
            strcpy(cmd->args[cmd->num_args], token);
            cmd->num_args++;
        }
    }
    
    return 1; // Comando parseado com sucesso
}

// ========================================
// FUNÇÕES DE EXECUÇÃO DE COMANDOS
// ========================================

// Função para executar comando "criar_dir"
int executarCriarDir(comando_automatico* cmd) {
    if (cmd->num_args < 1) {
        printf("❌ Erro: criar_dir requer 1 argumento (nome)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("📁 Executando: criar_dir %s\n", cmd->args[0]);
    }
    
    int resultado = criarDiretorio(&sistema, cmd->args[0], diretorio_atual);
    if (resultado != -1) {
        printf("✅ Diretório '%s' criado com sucesso\n", cmd->args[0]);
        return 0;
    } else {
        printf("❌ Falha ao criar diretório '%s'\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "criar_arquivo"
int executarCriarArquivo(comando_automatico* cmd) {
    if (cmd->num_args < 1) {
        printf("❌ Erro: criar_arquivo requer 1 argumento (nome)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("📄 Executando: criar_arquivo %s\n", cmd->args[0]);
    }
    
    int resultado = criarArquivo(&sistema, cmd->args[0], diretorio_atual);
    if (resultado != -1) {
        printf("✅ Arquivo '%s' criado com sucesso\n", cmd->args[0]);
        return 0;
    } else {
        printf("❌ Falha ao criar arquivo '%s'\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "navegar"
int executarNavegar(comando_automatico* cmd) {
    if (cmd->num_args < 1) {
        printf("❌ Erro: navegar requer 1 argumento (diretório)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("🔄 Executando: navegar %s\n", cmd->args[0]);
    }
    
    int resultado;
    if (strcmp(cmd->args[0], "/") == 0) {
        irParaRaiz();
        resultado = 0;
    } else if (cmd->args[0][0] == '/') {
        resultado = navegarParaCaminhoCompleto(&sistema, cmd->args[0]);
    } else {
        resultado = navegarPara(&sistema, cmd->args[0]);
    }
    
    if (resultado == 0) {
        printf("✅ Navegou para '%s'\n", cmd->args[0]);
        return 0;
    } else {
        printf("❌ Falha ao navegar para '%s'\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "listar"
int executarListar(comando_automatico* cmd) {
    if (mostrar_comandos) {
        printf("📋 Executando: listar\n");
    }
    
    printf("═══ Conteúdo do diretório atual ═══\n");
    listarDiretorio(&sistema, diretorio_atual);
    return 0;
}

// Função para executar comando "importar"
int executarImportar(comando_automatico* cmd) {
    if (cmd->num_args < 2) {
        printf("❌ Erro: importar requer 2 argumentos (nome_no_simulador caminho_arquivo_real)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("📥 Executando: importar %s %s\n", cmd->args[0], cmd->args[1]);
    }
    
    // Criar arquivo no simulador
    int inode_arquivo = criarArquivo(&sistema, cmd->args[0], diretorio_atual);
    if (inode_arquivo != -1) {
        // Importar conteúdo
        if (importarArquivo(&sistema, inode_arquivo, cmd->args[1]) == 0) {
            printf("✅ Arquivo '%s' importado com sucesso\n", cmd->args[0]);
            return 0;
        } else {
            printf("❌ Falha ao importar conteúdo do arquivo '%s'\n", cmd->args[1]);
            return -1;
        }
    } else {
        printf("❌ Falha ao criar arquivo '%s' no simulador\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "renomear"
int executarRenomear(comando_automatico* cmd) {
    if (cmd->num_args < 2) {
        printf("❌ Erro: renomear requer 2 argumentos (nome_atual nome_novo)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("✏️ Executando: renomear %s %s\n", cmd->args[0], cmd->args[1]);
    }
    
    if (renomearEntrada(&sistema, diretorio_atual, cmd->args[0], cmd->args[1]) == 0) {
        printf("✅ '%s' renomeado para '%s'\n", cmd->args[0], cmd->args[1]);
        return 0;
    } else {
        printf("❌ Falha ao renomear '%s'\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "mover"
int executarMover(comando_automatico* cmd) {
    if (cmd->num_args < 2) {
        printf("❌ Erro: mover requer 2 argumentos (nome_arquivo caminho_destino)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("🔄 Executando: mover %s %s\n", cmd->args[0], cmd->args[1]);
    }
    
    if (moverArquivoComCaminho(&sistema, diretorio_atual, cmd->args[0], cmd->args[1]) == 0) {
        return 0; // Mensagem já é exibida pela função
    } else {
        return -1;
    }
}

// Função para executar comando "apagar"
int executarApagar(comando_automatico* cmd) {
    if (cmd->num_args < 1) {
        printf("❌ Erro: apagar requer 1 argumento (nome)\n");
        return -1;
    }
    
    if (mostrar_comandos) {
        printf("🗑️ Executando: apagar %s\n", cmd->args[0]);
    }
    
    // Verificar se é arquivo ou diretório
    int inode = buscarEntradaDiretorio(&sistema, diretorio_atual, cmd->args[0]);
    if (inode == -1) {
        printf("❌ '%s' não encontrado\n", cmd->args[0]);
        return -1;
    }
    
    int resultado;
    if (sistema.inodes[inode].tipo == 0) {
        // É um arquivo
        resultado = apagarArquivo(&sistema, diretorio_atual, cmd->args[0]);
    } else {
        // É um diretório
        resultado = apagarDiretorio(&sistema, diretorio_atual, cmd->args[0]);
    }
    
    if (resultado == 0) {
        printf("✅ '%s' apagado com sucesso\n", cmd->args[0]);
        return 0;
    } else {
        printf("❌ Falha ao apagar '%s'\n", cmd->args[0]);
        return -1;
    }
}

// Função para executar comando "info"
int executarInfo(comando_automatico* cmd) {
    if (mostrar_comandos) {
        printf("📊 Executando: info\n");
    }
    
    printf("═══ Informações do Sistema ═══\n");
    mostrarEstatisticas(&sistema);
    return 0;
}

// Função para executar comando "echo"
int executarEcho(comando_automatico* cmd) {
    printf("💬 ");
    for (int i = 0; i < cmd->num_args; i++) {
        printf("%s ", cmd->args[i]);
    }
    printf("\n");
    return 0;
}

// Função para executar comando "pausar"
int executarPausar(comando_automatico* cmd) {
    if (mostrar_comandos) {
        printf("⏸️ Executando: pausar\n");
    }
    
    printf("⏸️ Pressione Enter para continuar...");
    getchar();
    return 0;
}

// ========================================
// FUNÇÃO PRINCIPAL DE EXECUÇÃO
// ========================================

// Função para executar um comando automaticamente
int executarComandoAutomatico(comando_automatico* cmd) {
    if (!cmd || strlen(cmd->comando) == 0) {
        return 0; // Comando vazio
    }
    
    // Mapear comandos para funções
    if (strcmp(cmd->comando, "criar_dir") == 0) {
        return executarCriarDir(cmd);
    } else if (strcmp(cmd->comando, "criar_arquivo") == 0) {
        return executarCriarArquivo(cmd);
    } else if (strcmp(cmd->comando, "navegar") == 0) {
        return executarNavegar(cmd);
    } else if (strcmp(cmd->comando, "listar") == 0) {
        return executarListar(cmd);
    } else if (strcmp(cmd->comando, "importar") == 0) {
        return executarImportar(cmd);
    } else if (strcmp(cmd->comando, "renomear") == 0) {
        return executarRenomear(cmd);
    } else if (strcmp(cmd->comando, "mover") == 0) {
        return executarMover(cmd);
    } else if (strcmp(cmd->comando, "apagar") == 0) {
        return executarApagar(cmd);
    } else if (strcmp(cmd->comando, "info") == 0) {
        return executarInfo(cmd);
    } else if (strcmp(cmd->comando, "echo") == 0) {
        return executarEcho(cmd);
    } else if (strcmp(cmd->comando, "pausar") == 0) {
        return executarPausar(cmd);
    } else {
        printf("❌ Comando desconhecido: '%s'\n", cmd->comando);
        return -1;
    }
}

// ========================================
// FUNÇÃO PARA PROCESSAR ARQUIVO DE COMANDOS
// ========================================

// Função para executar comandos de um arquivo
int executarArquivoComandos(const char* caminho_arquivo) {
    FILE* arquivo = fopen(caminho_arquivo, "r");
    if (!arquivo) {
        printf("❌ Erro ao abrir arquivo de comandos: %s\n", caminho_arquivo);
        return -1;
    }
    
    char linha[MAX_LINHA_COMANDO];
    comando_automatico cmd;
    int linha_num = 0;
    int comandos_executados = 0;
    int comandos_com_erro = 0;
    
    printf("🚀 Iniciando execução automática de comandos...\n");
    printf("📁 Arquivo: %s\n", caminho_arquivo);
    printf("═══════════════════════════════════════════\n");
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        linha_num++;
        
        int resultado_parse = parsearComando(linha, &cmd);
        
        if (resultado_parse == 0) {
            // Linha vazia ou comentário, continua
            continue;
        } else if (resultado_parse == -1) {
            printf("⚠️ Linha %d: Erro no parsing\n", linha_num);
            continue;
        }
        
        // Mostra a localização atual se habilitado
        if (mostrar_comandos) {
            construirCaminhoString();
            printf("[%s] ", caminho_string);
        }
        
        // Executa o comando
        int resultado = executarComandoAutomatico(&cmd);
        comandos_executados++;
        
        if (resultado != 0) {
            comandos_com_erro++;
            printf("❌ Erro na linha %d: %s\n", linha_num, linha);
            
            if (pausar_em_erro) {
                printf("⏸️ Pausar execução? (s/N): ");
                char resposta;
                scanf("%c", &resposta);
                limparBuffer();
                
                if (resposta == 's' || resposta == 'S') {
                    printf("⏹️ Execução interrompida pelo usuário\n");
                    break;
                }
            }
        }
        
        // Pequena pausa para visualização
        if (mostrar_comandos) {
            usleep(100000); // 100ms
        }
    }
    
    fclose(arquivo);
    
    printf("═══════════════════════════════════════════\n");
    printf("✅ Execução automática concluída!\n");
    printf("📊 Estatísticas:\n");
    printf("   • Comandos executados: %d\n", comandos_executados);
    printf("   • Comandos com erro: %d\n", comandos_com_erro);
    printf("   • Taxa de sucesso: %.1f%%\n", 
           comandos_executados > 0 ? (float)(comandos_executados - comandos_com_erro) / comandos_executados * 100 : 0);
    
    return 0;
}

// ========================================
// MENU PARA EXECUÇÃO AUTOMÁTICA
// ========================================

// Menu para configurar e executar comandos automáticos
void menuExecucaoAutomatica() {
    int opcao;
    char caminho_arquivo[256];
    
    do {
        printf("\n═══════════════════════════════════════════\n");
        printf("🤖 EXECUÇÃO AUTOMÁTICA DE COMANDOS\n");
        printf("═══════════════════════════════════════════\n");
        printf("Configurações atuais:\n");
        printf("  • Mostrar comandos: %s\n", mostrar_comandos ? "SIM" : "NÃO");
        printf("  • Pausar em erro: %s\n", pausar_em_erro ? "SIM" : "NÃO");
        printf("\n1. 📄 Executar arquivo de comandos\n");
        printf("2. ⚙️ Alternar exibição de comandos\n");
        printf("3. 🛑 Alternar pausa em erro\n");
        printf("4. 📖 Ver comandos disponíveis\n");
        printf("5. 📝 Criar arquivo de exemplo\n");
        printf("0. ⬅️ Voltar ao menu principal\n");
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
                printf("\n📄 Executar arquivo de comandos\n");
                printf("Caminho do arquivo: ");
                fgets(caminho_arquivo, sizeof(caminho_arquivo), stdin);
                caminho_arquivo[strcspn(caminho_arquivo, "\n")] = 0;
                
                if (strlen(caminho_arquivo) > 0) {
                    executarArquivoComandos(caminho_arquivo);
                } else {
                    printf("❌ Caminho inválido!\n");
                }
                pausar();
                break;
                
            case 2:
                mostrar_comandos = !mostrar_comandos;
                printf("✅ Exibição de comandos %s\n", mostrar_comandos ? "ATIVADA" : "DESATIVADA");
                pausar();
                break;
                
            case 3:
                pausar_em_erro = !pausar_em_erro;
                printf("✅ Pausa em erro %s\n", pausar_em_erro ? "ATIVADA" : "DESATIVADA");
                pausar();
                break;
                
            case 4:
                printf("\n📖 COMANDOS DISPONÍVEIS\n");
                printf("═══════════════════════════════════════════\n");
                printf("📁 criar_dir <nome>                  - Criar diretório\n");
                printf("📄 criar_arquivo <nome>              - Criar arquivo vazio\n");
                printf("🔄 navegar <dir>                     - Navegar para diretório\n");
                printf("📋 listar                            - Listar conteúdo atual\n");
                printf("📥 importar <nome> <caminho>         - Importar arquivo externo\n");
                printf("✏️ renomear <antigo> <novo>          - Renomear arquivo/diretório\n");
                printf("🔄 mover <arquivo> <destino>         - Mover arquivo\n");
                printf("🗑️ apagar <nome>                     - Apagar arquivo/diretório\n");
                printf("📊 info                              - Mostrar estatísticas\n");
                printf("💬 echo <texto>                      - Exibir mensagem\n");
                printf("⏸️ pausar                            - Pausar execução\n");
                printf("\n💡 DICAS:\n");
                printf("• Use # para comentários\n");
                printf("• Caminhos absolutos começam com /\n");
                printf("• Use '..' para voltar um nível\n");
                printf("• Linhas vazias são ignoradas\n");
                pausar();
                break;
                
            case 5:
                printf("\n📝 Criar arquivo de exemplo\n");
                printf("Nome do arquivo a criar: ");
                fgets(caminho_arquivo, sizeof(caminho_arquivo), stdin);
                caminho_arquivo[strcspn(caminho_arquivo, "\n")] = 0;
                
                if (strlen(caminho_arquivo) > 0) {
                    FILE* exemplo = fopen(caminho_arquivo, "w");
                    if (exemplo) {
                        fprintf(exemplo, "# Arquivo de exemplo para execução automática\n");
                        fprintf(exemplo, "# Linhas que começam com # são comentários\n\n");
                        fprintf(exemplo, "echo Iniciando demonstração do sistema de arquivos\n");
                        fprintf(exemplo, "info\n\n");
                        fprintf(exemplo, "# Criar estrutura de diretórios\n");
                        fprintf(exemplo, "criar_dir documentos\n");
                        fprintf(exemplo, "criar_dir imagens\n");
                        fprintf(exemplo, "criar_dir programas\n\n");
                        fprintf(exemplo, "# Listar conteúdo da raiz\n");
                        fprintf(exemplo, "listar\n\n");
                        fprintf(exemplo, "# Navegar para documentos e criar arquivos\n");
                        fprintf(exemplo, "navegar documentos\n");
                        fprintf(exemplo, "criar_arquivo readme.txt\n");
                        fprintf(exemplo, "criar_arquivo notas.txt\n");
                        fprintf(exemplo, "listar\n\n");
                        fprintf(exemplo, "# Voltar para raiz\n");
                        fprintf(exemplo, "navegar /\n");
                        fprintf(exemplo, "echo Demonstração concluída!\n");
                        
                        fclose(exemplo);
                        printf("✅ Arquivo de exemplo criado: %s\n", caminho_arquivo);
                    } else {
                        printf("❌ Erro ao criar arquivo de exemplo\n");
                    }
                } else {
                    printf("❌ Nome inválido!\n");
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

// ========================================
// MODIFICAÇÃO NO MENU PRINCIPAL
// ========================================

// Esta função deve ser integrada ao menuPrincipal() existente
// Adicione este case no switch do menu principal:
/*
case 5:
    if (sistema_inicializado) {
        menuExecucaoAutomatica();
    } else {
        printf("❌ Sistema não inicializado!\n");
        pausar();
    }
    break;
*/

// E modifique a lista de opções para incluir:
// printf("5. 🤖 Execução automática de comandos\n");
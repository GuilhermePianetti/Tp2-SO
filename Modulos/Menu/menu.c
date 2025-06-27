#include "menu.h"

// Definição das variáveis globais (alocação de memória real)


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
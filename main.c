#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "./Modulos/Particao/particao.h"
#include "./Modulos/Diretorio/diretorio.h"
#include "./Modulos/Arquivo/arquivo.h"
#include "./Modulos/Menu/menu.h"




// Função main
int main() {
    printf("🗄️  Simulador de Sistema de Arquivos\n");
    printf("═══════════════════════════════════════════\n\n");
    
    // Inicializar variáveis globais
    memset(&sistema, 0, sizeof(particao));
    diretorio_atual = 0;
    sistema_inicializado = false;
    modo_verboso = true;
    
    // Executar menu principal
    menuPrincipal();
    
    return 0;
}
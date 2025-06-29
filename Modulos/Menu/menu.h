#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../Outros/caminho.h"  
#include "../Outros/globais.h"

//fubcoes auxiliares
void exibirInformacoesVerbosas(const char* operacao);
void exibirEstadoInodes(const char* contexto);

// Menus principais
void menuPrincipal(void);
void menuInicializarSistema(void);
void menuDiretorios(void);
void menuArquivos(void);
void menuConfiguracoes(void);

#endif // MENU_H

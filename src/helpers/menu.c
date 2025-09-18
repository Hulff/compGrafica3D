// Gera a UI de inicio

#include <stdio.h>
#include <stdlib.h>
#include "clear.h"

// Gera a UI
void startUI(int *option)
{
    int temp;
    clearScreen();
    printf("Seja bem-vindo ao Paint 2025 atualizado Premium!\n");
    printf("############################################\n");
    printf("Pressione '1' iniciar o modo de desenho livre\n");
    printf("Pressione '2' para sair.\n");
    printf("############################################\n");
    while (1)
    {
        printf("Escolha uma opcao: ");
        if (scanf("%d", &temp) == 1 && temp >= 1 && temp <= 2) // scanf("%d", &temp) retorna 1 se a entrada foi válida
        {
            *option = temp;
            break;
        }
        else
        {
            printf("Opcao inválida. Tente novamente.\n");
            while (getchar() != '\n')
                ;
        }
    }
}

void programUI()

{
    clearScreen();
    printf("############################################\n");
    printf("Controles:\n");
    printf("W/S ou ↑/↓: mover para frente/para trás\n");
    printf("A/D ou ←/→: virar para esquerda/direita\n");
    printf("Mouse: olhar ao redor\n");
    printf("Clique direito: travar/destravar controle do mouse\n");
    printf("############################################\n");
}
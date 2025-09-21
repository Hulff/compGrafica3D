// Gera a UI de inicio

#include <stdio.h>
#include <stdlib.h>
#include "clear.h"

// Gera a UI
void startUI(int *option)
{
    int temp;
    clearScreen();
    printf("Seja bem-vindo ao melhor jogo 3D de 2025 \n");
    printf("############################################\n");
    printf("Pressione '1' iniciar o jogo\n");
    printf("Pressione '2' para sair, mas porque? tá com medinho?\n");
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
    printf("W/S  mover para frente/para trás/esquerda/direita\n");
    printf("A/D  mover para esquerda/direita\n");
    printf("seta ↑/↓: virar para cima/baixo\n");
    printf("seta ←/→: virar para esquerda/direita\n");
    printf("Mouse: olhar ao redor\n");
    printf("Clique direito: travar/destravar controle do mouse\n");
    printf("############################################\n");
}
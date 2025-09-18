// detecta teclado e mouse

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shape.h"
#include "storage.h"
#include "menu.h"
#include "config.h"

extern float r, g, b;
extern ShapeStack *storage; // pilha global de figuras (criado na Main)

typedef enum
{
    TRANSLATE,
    ROTATE,
    SCALE,
    SHEAR,
    COLOR,
    NONE,
    REFLECT,
    SELECTION,
} Operation;

typedef enum
{
    SHEAR_HORIZONTAL,
    SHEAR_VERTICAL
} ShearType;

bool waitingForClick = false; // controla captura de ponto
bool createShapeMode = false; // controla criação de forma

int n_points = 0; // número de pontos criados

Operation currentOperation; // guarda qual operação está sendo feita

int selectedColorPos = 0; // guarda a posição da cor escolhida
float colors[8][3] = {
    {0.0f, 0.0f, 0.0f}, // preto
    {1.0f, 0.0f, 0.0f}, // vermelho
    {0.0f, 1.0f, 0.0f}, // verde
    {0.0f, 0.0f, 1.0f}, // azul
    {1.0f, 1.0f, 0.0f}, // amarelo
    {1.0f, 0.0f, 1.0f}, // magenta
    {0.0f, 1.0f, 1.0f}, // ciano
    {1.0f, 1.0f, 1.0f}, // branco
};
// helper para resetar estados
void resetStates()
{
    waitingForClick = false; // cancelar captura de ponto
    createShapeMode = false; // cancelar modo de criação de forma
    currentOperation = NONE; // cancelar operação atual
    n_points = 0;            // reseta o numero de pontos
}

// ler teclado
void teclado(unsigned char key, int x, int y)
{
    printf("Tecla: %c\n", key);

    switch (key)
    {
    case 'b':
        r = 0;
        g = 0;
        b = 0;
        break;
    case 'w':
        r = 1;
        g = 1;
        b = 1;
        break;
    case 'q':
    }

    glutPostRedisplay();
}
// ler as setas (sem uso ainda)
void tecladoEspecial(int key, int x, int y)
{
    printf("Tecla especial: %d\n", key);
    if (key == GLUT_KEY_UP)
    {
        printf("Seta ↑\n");
    }
    if (key == GLUT_KEY_DOWN)
    {
        printf("Seta ↓\n");
    }
}

// ler pos do clique do mouse
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        resetStates(); // cancelar captura de ponto
    }
}
// ler pos do mouse sempre
void mouseMove(int x, int y)
{
}

void mouseWheel(int wheel, int direction, int x, int y)
{
}

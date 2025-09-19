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
extern float alpha, beta, delta;
extern float movement; // movimento da câmera
extern float camX, camY, camZ; // posição do jogador
// extern float xpos, ypos; // posição da camera

float mouseSensitivity = 0.3f; // ajuste entre 0.01 (muito lento) e 1.0 (rápido)

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

int n_points = 0;         // número de pontos criados
int lockMouseControl = 0; // trava o controle do mouse

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
    currentOperation = NONE; // cancelar operação atual
    n_points = 0;            // reseta o numero de pontos
}

void moveForward()
{
    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    float len = sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    dirX /= len;
    dirY /= len;
    dirZ /= len;

    camX += dirX * movement;
    camY += dirY * movement;
    camZ += dirZ * movement;
}
void moveBackwards()
{
    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    float len = sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    dirX /= len;
    dirY /= len;
    dirZ /= len;

    camX += dirX * -movement;
    camY += dirY * -movement;
    camZ += dirZ * -movement;
}

// ler teclado
void teclado(unsigned char key, int x, int y)
{
    // printf("Tecla: %c\n", key);

    switch (key)
    {
    case 'b':
        r = 0;
        g = 0;
        b = 0;
        break;
    case 'w':
        moveForward();
        break;
    case 's':
        moveBackwards();
        break;
    case 'a':
        beta -= 0.1f;
        break;
    case 'd':
        beta += 0.1f;
        break;
    }

    glutPostRedisplay();
}
// ler as setas (sem uso ainda)
void tecladoEspecial(int key, int x, int y)
{
    // printf("Tecla especial: %d\n", key);
    switch (key)
    {
    case GLUT_KEY_UP:
        moveForward();
        break;
    case GLUT_KEY_DOWN:
        moveBackwards();
        break;
    case GLUT_KEY_LEFT:
        beta -= 0.1f;
        break;
    case GLUT_KEY_RIGHT:
        beta += 0.1f;
        break;
    }
    programUI();
    glutPostRedisplay();
}

// ler pos do clique do mouse
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        lockMouseControl = !lockMouseControl; // trava/destrava o controle do mouse
        if (lockMouseControl)
            printf("Mouse control locked\n");
        else
            printf("Mouse control unlocked\n");
    }
}
// ler pos do mouse sempre
void mouseMove(int x, int y)
{

    if (lockMouseControl)
    {
        // converter coordenadas da tela para coordenadas do mundo

        float worldX = (x / (float)windW) * 10 - 5;           // -5 a 5
        float worldY = ((windH - y) / (float)windH) * 10 - 5; // -5 a 5
        // printf("Mouse move (mundo): %.2f, %.2f\n", worldX, worldY);

        alpha = worldY * mouseSensitivity;
        beta = worldX * mouseSensitivity;
        programUI();
        
        glutPostRedisplay();
    }
}

void mouseWheel(int wheel, int direction, int x, int y)
{
    if (direction > 0)
    {
        delta *= 1.1f; // zoom in
    }
    else
    {
        delta *= 0.9f; // zoom out
    }

    glutPostRedisplay();
}


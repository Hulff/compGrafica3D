
#include <GL/freeglut.h>
#include <stdbool.h>
#include "menu.h"
#include "storage.h"
#include "shape.h"
#include "config.h"
#include "input.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SPEED 0.01f
#define NUM_RINGS 20
#define NUM_BUILDINGS 20

typedef struct
{
    float x, y, z;
} Structure;
Structure rings[NUM_RINGS];
Structure buildings[NUM_BUILDINGS];

// Definição das variáveis globais
float r = 1.0f, g = 1.0f, b = 1.0f;
float alpha = 0.0f, beta = 0.0f, delta = 1.0f; // ângulos de rotação e zoom
float camX = 0, camY = 0, camZ = 0;            // posição do jogador

float movement = 0.1f; // velocidade de movimento da câmera
//implementar boost temporario depois de passar por anel


void initRings()
{
    for (int i = 0; i < NUM_RINGS; i++)
    {
        rings[i].x = (rand() % 20-10) / 2.0f;
        rings[i].y = (rand() % 20) / 2.0f;
        rings[i].z = -(float)(i + 1) * 10.0f;
    }
}

void initBuildings()
{
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        buildings[i].x = (rand() % 20 - 10) / 2.0f; // parecido com os anéis
        buildings[i].z = -(float)(i + 1) * 7.0f;   // mesma distância incremental
        buildings[i].y = 0.0f;
    }
}

void drawGround()
{
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);
    glColor3f(0.3f, 1.0f, 0.3f); // cor verde para o chão
    glBegin(GL_QUADS);
    glVertex3f(-250.0f, 0.0f, -250.0f);
    glVertex3f(250.0f, 0.0f, -250.0f);
    glVertex3f(250.0f, 0.0f, 250.0f);
    glVertex3f(-250.0f, 0.0f, 250.0f);
    glEnd();
    glPopMatrix();
}

void drawRings()
{
    for (int i = 0; i < NUM_RINGS; i++)
    {
        {
            glPushMatrix();
            glTranslatef(rings[i].x, rings[i].y, rings[i].z);
            glColor3f(0.5, 0.5, 1.0);
            glutSolidTorus(0.05, 1.0, 20, 60);
            glPopMatrix();
        }
    }
}

void drawScenario()
{
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        glPushMatrix();
        glTranslatef(buildings[i].x, -2.0f, buildings[i].z);
        glScalef(1.0f, 2.0f, 1.0f);
        glColor3f(0.6f, 0.6f, 0.7f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

void init(void)
{
    glClearColor(r, g, b, 0);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)windW / windH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    initRings();
    initBuildings();
};

void display()
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    gluLookAt(camX, camY, camZ,camX + dirX, camY + dirY, camZ + dirZ,0, 1, 0);

    //desenhar player

    drawGround();
    drawScenario();
    drawRings();
    glutSwapBuffers();
}

void idle()
{
}

int main(int argc, char **argv)
{
    int option;
    bool control = true;

    while (control)
    {
        startUI(&option);
        switch (option)
        {
        case 1:
            control = false;
            break;
        case 2:
            return 0;
        default:
            break;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(windW, windH);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("jogao 3D muito divertido merece 10");

    init();
    glutDisplayFunc(display);

    glutKeyboardFunc(teclado);
    glutSpecialFunc(tecladoEspecial);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMove);
    glutMouseWheelFunc(mouseWheel);
    programUI();

    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}

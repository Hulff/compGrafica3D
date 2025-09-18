
#include <GL/freeglut.h>
#include <stdbool.h>
#include "menu.h"
#include "storage.h"
#include "shape.h"
#include "config.h"
#include "input.h"
#include <stdio.h>

#define SPEED 0.01f

// Definição das variáveis globais
float r = 1.0f, g = 1.0f, b = 1.0f;
float alpha = 0.0f, beta = 0.0f, delta = 1.0f; // ângulos de rotação e zoom

float xpos = 0.0f, ypos = 0.0f, zpos = 0.0f; // posição da camera

//



void init(void)
{
    glClearColor(r, g, b, 0);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)windW / windH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
};
void display()
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
    
    glColor3f(1.0, 0.0, 0.0); // cor do cubo

    glTranslatef(0.0f, 0.0f, -5.0f); // afastar o cubo da camera
    glRotatef(alpha, 1.0f, 0.0f, 0.0f);
    glRotatef(beta, 0.0f, 1.0f, 0.0f);
    glScalef(delta, delta, delta);

    glutSolidCube(1.0);


    glutSwapBuffers();
}

void idle()
{
    // xpos += SPEED * 0.01f;
    // ypos += SPEED * 0.01f;
    // zpos += SPEED * 0.01f;
    // glutPostRedisplay();
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

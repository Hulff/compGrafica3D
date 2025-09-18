
#include <GL/freeglut.h>
#include <stdbool.h>
#include "menu.h"
#include "storage.h"
#include "shape.h"
#include "config.h"
#include "input.h"



// Definição das variáveis globais
float r = 1.0f, g = 1.0f, b = 1.0f;
ShapeStack *storage;
//

void init(void)
{
    glClearColor(r, g, b, 0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, windW, 0, windH);
};
void display()
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
   
    glFlush();
}

int main(int argc, char **argv)
{
    int option;
    bool control = true;
    storage = criarPilha(maxFig); 


    while (control)
    {
        startUI(&option);
        switch (option)
        {
        case 1: 
            control = false;
            break;
        case 3:
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

    glutMainLoop();
    return 0;
}

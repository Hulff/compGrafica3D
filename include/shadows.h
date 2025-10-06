#ifndef SHADOWS_H
#define SHADOWS_H

#include <GL/freeglut.h>

// Inicialização das sombras
void initShadows(void);

// Matriz de projeção de sombra
void applyShadowMatrix(const GLfloat lightPos[4], const GLfloat groundPlane[4]);

// Renderiza sombras
void drawSceneShadow(void (*drawGeometryFunc)(void),
                     const GLfloat lightPos[4],
                     const GLfloat groundPlane[4]);

#endif

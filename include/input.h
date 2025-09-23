#ifndef INPUT_H
#define INPUT_H

// input.h
extern float alpha, beta, camX, camY, camZ, movement;
extern float r, g, b;
extern double lastTime;
extern float r, g, b;
void teclado(unsigned char key, int x, int y);
void tecladoEspecial(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMove(int x, int y);
void mouseWheel(int wheel, int direction, int x, int y);

#endif

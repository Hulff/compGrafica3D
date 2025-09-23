
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

#include <time.h>

#define SPEED 0.01f
#define NUM_RINGS 20
#define NUM_BUILDINGS 20

// texturas
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct
{
    float x, y, z;
    bool passed;
} Structure;

float scaleFactors[NUM_BUILDINGS];

Structure rings[NUM_RINGS];
Structure buildings[NUM_BUILDINGS];

// Definição das variáveis globais
float r = 1.0f, g = 1.0f, b = 1.0f;
float alpha = 0.0f, beta = 0.0f, delta = 1.0f; // ângulos de rotação e zoom
float camX = 0, camY = 0, camZ = 0;            // posição do jogador

float movement = 0.1f; // velocidade de movimento da câmera

int lastRingIndex = -1; // índice do último anel passado

// variaveis de controle de tempo / frame rate
const float fps = 60.0f;
const float frameDelay = 1.0f / fps; // segundos
double lastTime = 0.0;
bool wrongRing = false;
// TODO adicionar iluminação

//
GLuint textureID;
GLuint groundTexture = 0; // <-- textura do chão

GLuint loadTexture(const char *filename)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data)
    {
        printf("Erro ao carregar imagem: %s\n", filename);
        return 0; // retorna 0 em caso de erro
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // parâmetros de repetição e filtros
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height,
                      format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // desliga a textura

    return texID; // retorna o ID da textura gerada
}

double getTime()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

// auxiliares
void initRings()
{
    for (int i = 0; i < NUM_RINGS; i++)
    {
        rings[i].x = (rand() % 20 - 10) / 2.0f;
        rings[i].y = (rand() % 20) / 2.0f;
        rings[i].z = -(float)(i + 1) * 10.0f;
    }
}

void initBuildings()
{
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        buildings[i].x = (rand() % 20 - 10) / 2.0f; // parecido com os anéis
        buildings[i].z = -(float)(i + 1) * 7.0f;    // mesma distância incremental
        buildings[i].y = 0.0f;
        scaleFactors[i] = (rand() % 3) + 2.0f; // fator de escala aleatório entre 2  e 5
    }
}

void drawGround()
{
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);

    if (groundTexture != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    int repeat = 120; // número de repetições no X e Y

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-250.0f, 0.0f, -250.0f);
    glTexCoord2f(repeat, 0.0f);
    glVertex3f(250.0f, 0.0f, -250.0f);
    glTexCoord2f(repeat, repeat);
    glVertex3f(250.0f, 0.0f, 250.0f);
    glTexCoord2f(0.0f, repeat);
    glVertex3f(-250.0f, 0.0f, 250.0f);

    glEnd();

    if (groundTexture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

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
        float scale = scaleFactors[i];
        glScalef(1.2f, scale * 1.0f, 1.0f);
        glColor3f(0.6f, 0.6f, 0.7f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

// principais
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
    // carregar textura do chão (arquivo deve existir)
    groundTexture = loadTexture("textures/grass.jpg"); // coloque sua imagem "grass.jpg" na pasta do executável
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

    gluLookAt(camX, camY, camZ,
              camX + dirX, camY + dirY, camZ + dirZ,
              0, 1, 0);

    drawGround();
    drawScenario();
    drawRings();

    // check de passagem nos aneis
    for (int i = 0; i < NUM_RINGS; i++)
    {
        float dx = camX - rings[i].x;
        float dy = camY - rings[i].y;
        float dz = camZ - rings[i].z;
        float dist2 = dx * dx + dy * dy + dz * dz;

        float threshold = 1.0f;

        // se tentou passar fora de ordem
        if (i != lastRingIndex + 1 && i != lastRingIndex &&
            dist2 < threshold * threshold &&
            !rings[i].passed && !wrongRing)
        {
            movement = 0.1f; // reduz a velocidade
            printf("Passe pelo anel %d antes de passar pelo anel %d\n",
                   lastRingIndex + 2, i + 1);

            wrongRing = true; // trava até acertar o próximo
            continue;
        }

        // se passou pelo anel correto
        if (i == lastRingIndex + 1 &&
            dist2 < threshold * threshold &&
            !rings[i].passed)
        {
            movement = 0.25f;  // aumenta a velocidade
            lastRingIndex = i; // atualiza o último anel passado
            rings[i].passed = true;
            wrongRing = false; // libera novas mensagens

            printf("Velocidade Maxima atingida: %.2f\n", movement);

            if (i == NUM_RINGS - 1)
            {
                printf("Voce passou por todos os aneis! Parabens!\n");
            }
            else
            {
                printf("Aneis restantes: %d\n", NUM_RINGS - i - 1);
            }
        }
    }

    glutSwapBuffers();
}

void idle()
{
    double currentTime = getTime();
    double delta = currentTime - lastTime;

    if (delta >= frameDelay)
    {
        // lógica do jogo
        if (movement > 0.1f)
        {
            movement -= 0.0005f;
            if (movement < 0.1f)
            {

                movement = 0.1f;
                printf("Velocidade Restaurada \n");
            }
        }

        lastTime = currentTime;
    }
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

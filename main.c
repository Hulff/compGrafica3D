
#include "menu.h"
#include "config.h"
#include "input.h"
#include "shadows.h"

#include <GL/freeglut.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SPEED 0.01f
#define NUM_RINGS 10
#define NUM_BUILDINGS 100
#define GROUND_Y (-2.0f)
#define PLANE_HALF 0.005f
#define NUM_PARTICLES 100

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
float r = 0.53f, g = 0.81f, b = 0.92f;
float alpha = 0.0f, beta = 0.0f, delta = 1.0f; // ângulos de rotação e zoom
float camX = 0, camY = 5.0f, camZ = 0;         // posição da camera
float playerX = 0, playerY = 0, playerZ = 0;   // posição do player
bool timerRunning = false;                     // inicia como false, só roda depois do countdown
double lastElapsed = 0.0;                      // guarda o último tempo decorrido quando o cronômetro para
double countdownStart = 0;                     // momento em que o countdown começou
double countdownTime = 3.0;                    // duração do countdown em segundos
bool countdownFinished = false;
bool canMove = false;                          // false enquanto o countdown não terminar
int lockMouseControl = 0;                      // toggle do controle via mouse
bool explosionActive = false;                  // flag para indicar se a explosão está ativa
double explosionStartTime = 0;                 // tempo de início da explosão

float movement = 0.1f; // velocidade de movimento da câmera

int lastRingIndex = -1; // índice do último anel passado

// variaveis de controle de tempo / frame rate
const float fps = 60.0f;
const float frameDelay = 1.0f / fps; // segundos
double lastTime = 0.0;
static double startTime = 0;
bool wrongRing = false;
// TODO adicionar iluminação

// importar o modelo 3D
// Carregar modelo
typedef struct
{
    float x, y, z;
} Vec3;

typedef struct
{
    float u, v;
} Vec2;

typedef struct
{
    int v[3];  // índices de vértices
    int vt[3]; // índices de texcoords (ou -1)
    int vn[3]; // índices de normais  (ou -1)
} Face;

typedef struct {
    float x, y, z; // posição
    float vx, vy, vz; // velocidade
    bool active; // se tá ativa
} Particle;

Particle particles[NUM_PARTICLES];             // partículas de explosão

typedef struct
{
    Vec3 *vertices;
    Vec3 *normals;
    Vec2 *texcoords;
    Face *faces;
    size_t numVertices;
    size_t numNormals;
    size_t numTexcoords;
    size_t numFaces;
} OBJModel;

OBJModel model = {0};

int loadOBJ(const char *filename, OBJModel *model)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return 0;
    }

    char line[512];

    // --- Primeiro passe: contar elementos ---
    size_t vCount = 0, vnCount = 0, vtCount = 0, fCount = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "v ", 2) == 0)
            vCount++;
        else if (strncmp(line, "vn ", 3) == 0)
            vnCount++;
        else if (strncmp(line, "vt ", 3) == 0)
            vtCount++;
        else if (strncmp(line, "f ", 2) == 0)
        {
            // Conta faces aproximado (cada polígono vira N-2 triângulos)
            int tokens = 0;
            for (char *p = line; *p; p++)
                if (*p == ' ')
                    tokens++;
            if (tokens >= 3)
                fCount += tokens - 2;
        }
    }

    // alocar memória
    model->vertices = malloc(sizeof(Vec3) * vCount);
    model->normals = malloc(sizeof(Vec3) * vnCount);
    model->texcoords = malloc(sizeof(Vec2) * vtCount);
    model->faces = malloc(sizeof(Face) * fCount);

    model->numVertices = vCount;
    model->numNormals = vnCount;
    model->numTexcoords = vtCount;
    model->numFaces = fCount;

    // --- Segundo passe: carregar dados ---
    rewind(file);

    size_t vi = 0, vti = 0, vni = 0, fi = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "v ", 2) == 0)
        {
            sscanf(line, "v %f %f %f",
                   &model->vertices[vi].x,
                   &model->vertices[vi].y,
                   &model->vertices[vi].z);
            vi++;
        }
        else if (strncmp(line, "vn ", 3) == 0)
        {
            sscanf(line, "vn %f %f %f",
                   &model->normals[vni].x,
                   &model->normals[vni].y,
                   &model->normals[vni].z);
            vni++;
        }
        else if (strncmp(line, "vt ", 3) == 0)
        {
            sscanf(line, "vt %f %f",
                   &model->texcoords[vti].u,
                   &model->texcoords[vti].v);
            vti++;
        }
        else if (strncmp(line, "f ", 2) == 0)
        {
            // ---- Parse da face com N vértices ----
            int v[64], vt[64], vn[64]; // suporta até 64 vértices por face
            int count = 0;

            char *ptr = line + 2; // pular "f "
            while (*ptr && count < 64)
            {
                int vi_ = -1, vti_ = -1, vni_ = -1;
                if (sscanf(ptr, "%d/%d/%d", &vi_, &vti_, &vni_) == 3)
                {
                    // v/vt/vn
                }
                else if (sscanf(ptr, "%d//%d", &vi_, &vni_) == 2)
                {
                    // v//vn
                }
                else if (sscanf(ptr, "%d/%d", &vi_, &vti_) == 2)
                {
                    // v/vt
                }
                else if (sscanf(ptr, "%d", &vi_) == 1)
                {
                    // só v
                }

                if (vi_ != -1)
                {
                    v[count] = vi_ - 1;
                    vt[count] = (vti_ > 0) ? vti_ - 1 : -1;
                    vn[count] = (vni_ > 0) ? vni_ - 1 : -1;
                    count++;
                }

                // avança ptr para o próximo token
                while (*ptr && *ptr != ' ')
                    ptr++;
                while (*ptr == ' ')
                    ptr++;
            }

            // triangulação em fan: (v0,v[i],v[i+1])
            for (int i = 1; i < count - 1; i++)
            {
                Face f;
                f.v[0] = v[0];
                f.v[1] = v[i];
                f.v[2] = v[i + 1];
                f.vt[0] = vt[0];
                f.vt[1] = vt[i];
                f.vt[2] = vt[i + 1];
                f.vn[0] = vn[0];
                f.vn[1] = vn[i];
                f.vn[2] = vn[i + 1];
                model->faces[fi++] = f;
            }
        }
    }

    fclose(file);
    return 1;
}

void drawOBJ(OBJModel *model)
{
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < model->numFaces; i++)
    {
        Face f = model->faces[i];
        for (int j = 0; j < 3; j++)
        {
            if (f.vn[j] >= 0 && model->numNormals > 0)
                glNormal3f(model->normals[f.vn[j]].x,
                           model->normals[f.vn[j]].y,
                           model->normals[f.vn[j]].z);
            if (f.vt[j] >= 0 && model->numTexcoords > 0)
                glTexCoord2f(model->texcoords[f.vt[j]].u,
                             model->texcoords[f.vt[j]].v);
            glVertex3f(model->vertices[f.v[j]].x,
                       model->vertices[f.v[j]].y,
                       model->vertices[f.v[j]].z);
        }
    }
    glEnd();
}

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

// colisão AABB
bool aabb_intersect(float axMin, float axMax, float ayMin, float ayMax, float azMin, float azMax,
                    float bxMin, float bxMax, float byMin, float byMax, float bzMin, float bzMax)
{
    return (axMin <= bxMax && axMax >= bxMin) &&
           (ayMin <= byMax && ayMax >= byMin) &&
           (azMin <= bzMax && azMax >= bzMin);
}

// colisão com o chão
bool checkCollisionWithGround(float py, float halfPlane)
{
    return (py - halfPlane) <= GROUND_Y;
}

// colisão com prédios
bool checkCollisionWithBuildingIndex(int i, float px, float py, float pz, float halfPlane)
{
    float originalSizeY = 1.0f;
    float scaleY = scaleFactors[i];
    float baseY = -2.0f;

    float halfHeight = (originalSizeY * scaleY) / 2.0f;
    float byMin = baseY - halfHeight;
    float byMax = baseY + halfHeight;

    float width = 1.2f;
    float depth = 1.0f;

    float bx = buildings[i].x;
    float bz = buildings[i].z;

    float bxMin = bx - width / 2.0f;
    float bxMax = bx + width / 2.0f;
    float bzMin = bz - depth / 2.0f;
    float bzMax = bz + depth / 2.0f;

    float axMin = px - halfPlane;
    float axMax = px + halfPlane;
    float ayMin = py - halfPlane;
    float ayMax = py + halfPlane;
    float azMin = pz - halfPlane;
    float azMax = pz + halfPlane;

    return aabb_intersect(axMin, axMax, ayMin, ayMax, azMin, azMax,
                          bxMin, bxMax, byMin, byMax, bzMin, bzMax);
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
    float zStart = -100.0f; // ponto inicial na frente do jogador
    float zEnd = 0.0f;      // ponto final da área de prédios
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        buildings[i].x = (rand() % 40 - 20) / 2.0f;                // X aleatório
        buildings[i].z = zStart + (rand() % (int)(zEnd - zStart)); // Z aleatório dentro da faixa
        buildings[i].y = 0.0f;

        scaleFactors[i] = 5.0f + (rand() % 11); // altura aleatória
    }
}

void drawGround()
{
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);

    glDisable(GL_LIGHTING);

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

    glColor3f(1.0f, 1.0f, 1.0f); // branco para textura sem alteração de cor

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-250.0f, 0.0f, 250.0f);
    glTexCoord2f(repeat, 0.0f);
    glVertex3f(250.0f, 0.0f, 250.0f);
    glTexCoord2f(repeat, repeat);
    glVertex3f(250.0f, 0.0f, -250.0f);
    glTexCoord2f(0.0f, repeat);
    glVertex3f(-250.0f, 0.0f, -250.0f);
    glEnd();

    if (groundTexture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawRings()
{
    for (int i = 0; i < NUM_RINGS; i++)
    {
        glPushMatrix();
        glTranslatef(rings[i].x, rings[i].y, rings[i].z);

        if (i == lastRingIndex + 1 && !rings[i].passed)
            glColor3f(1.0, 1.0, 0.0); // amarelo para o próximo anel
        else
            glColor3f(0.5, 0.5, 1.0); // azul para os demais

        glutSolidTorus(0.05, 1.0, 20, 60);
        glPopMatrix();
    }
}

void drawRingsShadow()
{
    for (int i = 0; i < NUM_RINGS; i++)
    {
        glPushMatrix();
        glTranslatef(rings[i].x, rings[i].y, rings[i].z);

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // cor da sombra

        glutSolidTorus(0.05, 1.0, 20, 60);
        glPopMatrix();
    }
}
// desenha os predios manualmente
void desenhaPredioManual(float size)
{
    float s = size / 2.0f;

    glBegin(GL_QUADS);
    // frente
    glNormal3f(0, 0, 1);
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);
    // trás
    glNormal3f(0, 0, -1);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);
    // esquerda
    glNormal3f(-1, 0, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    // direita
    glNormal3f(1, 0, 0);
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glVertex3f(s, -s, s);
    // topo
    glNormal3f(0, 1, 0);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);
    // fundo
    glNormal3f(0, -1, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);
    glEnd();
}

void drawScenario()
{
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        glPushMatrix();
        glTranslatef(buildings[i].x, -2.0f, buildings[i].z);
        float scale = scaleFactors[i];
        glScalef(1.2f, scale, 1.0f);

        glColor3f(0.6f, 0.6f, 0.7f); // cor normal do prédio

        glCullFace(GL_BACK); // cull das faces de trás
        desenhaPredioManual(1.0f);

        glPopMatrix();
    }
}

void drawScenarioShadow()
{
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        glPushMatrix();
        glTranslatef(buildings[i].x, -2.0f, buildings[i].z);
        float scale = scaleFactors[i];
        glScalef(1.2f, scale, 1.0f);

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // cor da sombra

        glCullFace(GL_BACK); // cull das faces de trás
        desenhaPredioManual(1.0f);

        glPopMatrix();
    }
}

void drawPlayer()
{
    glPushMatrix();

    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    float distance = 1.5f; // distância à frente da câmera
    float px = camX + dirX * distance;
    float py = camY + dirY * distance;
    float pz = camZ + dirZ * distance;

    // atualiza a posição global do avião para colisão
    playerX = px;
    playerY = py;
    playerZ = pz;

    glTranslatef(px, py, pz);
    float adjustmentFactor = 1.4f; // fator para ajustar a rotação do modelo

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);

    float yRotation = (-beta * 180.0f / 3.14159f);

    glRotatef(yRotation, 0, 1, 0);

    float xRotation = (-alpha * 180.0f / 3.14159f) * adjustmentFactor;
    if (xRotation > 40)
        xRotation = 40;
    if (xRotation < -20)
        xRotation = -20;
    glRotatef(xRotation, 1, 0, 0);

    float roll = (beta * 180.0f / 3.14159f);
    if (roll > 30)
        roll = 30;
    if (roll < -30)
        roll = -30;
    glRotatef(roll * adjustmentFactor, 0, 0, 1);

    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(0.1f, 0.1f, 0.1f);

    drawOBJ(&model);

    glPopMatrix();
}

void drawPlayerShadow()
{
    glPushMatrix();

    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    float distance = 1.5f; // distância à frente da câmera
    float px = camX + dirX * distance;
    float py = camY + dirY * distance;
    float pz = camZ + dirZ * distance;

    // atualiza a posição global do avião para colisão
    playerX = px;
    playerY = py;
    playerZ = pz;

    glTranslatef(px, py, pz);
    float adjustmentFactor = 1.4f; // fator para ajustar a rotação do modelo

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);

    float yRotation = (-beta * 180.0f / 3.14159f);

    glRotatef(yRotation, 0, 1, 0);

    float xRotation = (-alpha * 180.0f / 3.14159f) * adjustmentFactor;
    if (xRotation > 40)
        xRotation = 40;
    if (xRotation < -20)
        xRotation = -20;
    glRotatef(xRotation, 1, 0, 0);

    float roll = (beta * 180.0f / 3.14159f);
    if (roll > 30)
        roll = 30;
    if (roll < -30)
        roll = -30;
    glRotatef(roll * adjustmentFactor, 0, 0, 1);

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // cor da sombra
    glScalef(0.1f, 0.1f, 0.1f);

    drawOBJ(&model);

    glPopMatrix();
}

// principais
void init(void)
{
    glClearColor(r, g, b, 0);
    glEnable(GL_DEPTH_TEST);

    // === iluminação ===
    glEnable(GL_LIGHTING);       // habilita sistema de luz
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHT0);         // ativa a luz 0
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL); // deixa glColor influenciar material
    glEnable(GL_CULL_FACE);      // ativa backface culling (desenha só faces visíveis)
    glFrontFace(GL_CCW);         // frente = anti-horário (padrão)
    initShadows();

    // parâmetros da luz
    GLfloat lightPos[] = {0.0f, 10.0f, 5.0f, 1.0f}; // posição (w=1 → pontual)
    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // material básico (pra objetos brilharem com especular)
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    // === fim iluminação ===

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (float)windW / windH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    initRings();
    initBuildings();
    // carregar textura do chão (arquivo deve existir)
    groundTexture = loadTexture("textures/grass.jpg"); // coloque sua imagem "grass.jpg" na pasta do executável
    loadOBJ("models/Jet_Lowpoly.obj", &model); // coloque seu modelo "jet.obj" na pasta do executável

    // Countdown
    startTime = getTime();      // marca o início do tempo total
    countdownStart = getTime(); // marca o início do countdown
    countdownFinished = false;

    glCullFace(GL_BACK);
};

void initShadows(void) { }

void applyShadowMatrix(const GLfloat lightPos[4], const GLfloat groundPlane[4]) {
    GLfloat dot = groundPlane[0]*lightPos[0] +
                  groundPlane[1]*lightPos[1] +
                  groundPlane[2]*lightPos[2] +
                  groundPlane[3]*lightPos[3]; // adicione w do plano

    GLfloat shadowMat[16];

    shadowMat[0]  = dot - lightPos[0] * groundPlane[0];
    shadowMat[4]  = -lightPos[0] * groundPlane[1];
    shadowMat[8]  = -lightPos[0] * groundPlane[2];
    shadowMat[12] = -lightPos[0] * groundPlane[3];

    shadowMat[1]  = -lightPos[1] * groundPlane[0];
    shadowMat[5]  = dot - lightPos[1] * groundPlane[1];
    shadowMat[9]  = -lightPos[1] * groundPlane[2];
    shadowMat[13] = -lightPos[1] * groundPlane[3];

    shadowMat[2]  = -lightPos[2] * groundPlane[0];
    shadowMat[6]  = -lightPos[2] * groundPlane[1];
    shadowMat[10] = dot - lightPos[2] * groundPlane[2];
    shadowMat[14] = -lightPos[2] * groundPlane[3];

    shadowMat[3]  = -lightPos[3] * groundPlane[0];
    shadowMat[7]  = -lightPos[3] * groundPlane[1];
    shadowMat[11] = -lightPos[3] * groundPlane[2];
    shadowMat[15] = dot - lightPos[3] * groundPlane[3];

    glMultMatrixf(shadowMat);
}

void drawSceneShadow(void (*drawGeometryFunc)(void),
                     const GLfloat lightPos[4],
                     const GLfloat groundPlane[4])
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Estado fixo para sombra
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    glPushMatrix();
        glTranslatef(0.0f, 0.01f, 0.0f); // evita z-fighting e reduz altura aparente
        applyShadowMatrix(lightPos, groundPlane);
        drawGeometryFunc();
    glPopMatrix();

    glDisable(GL_POLYGON_OFFSET_FILL);
    glPopAttrib();
}

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

    GLfloat lightPos[] = {0.0f, 20.0f, 20.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    GLfloat groundPlane[] = {0.0f, 1.0f, 0.0f, -GROUND_Y};

    glPushMatrix();
        drawGround();
        drawScenario();
        
        drawSceneShadow(drawPlayerShadow, lightPos, groundPlane);
        drawSceneShadow(drawScenarioShadow, lightPos, groundPlane);
        drawSceneShadow(drawRingsShadow, lightPos, groundPlane);
        drawRings();
    glPopMatrix();
    drawPlayer();

    bool collided = false;

    if (checkCollisionWithGround(playerY, PLANE_HALF))
    {
        collided = true;
        movement = 0.0f;
        timerRunning = false;
        lockMouseControl = 0;
    }

    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        if (checkCollisionWithBuildingIndex(i, playerX, playerY, playerZ, PLANE_HALF))
        {
            collided = true;
            movement = 0.0f;
            timerRunning = false;
            lockMouseControl = 0; // trava o controle do mouse
            break;
        }
    }

    for (int i = 0; i < NUM_RINGS; i++)
    {
        float dx = camX - rings[i].x;
        float dy = camY - rings[i].y;
        float dz = camZ - rings[i].z;
        float dist2 = dx * dx + dy * dy + dz * dz;

        float threshold = 2.0f;

        if (i != lastRingIndex + 1 && i != lastRingIndex &&
            dist2 < threshold * threshold &&
            !rings[i].passed && !wrongRing)
        {
            movement = 0.1f;
            wrongRing = true;
            continue;
        }

        if (i == lastRingIndex + 1 &&
            dist2 < threshold * threshold &&
            !rings[i].passed)
        {
            movement = 0.25f;
            lastRingIndex = i;
            rings[i].passed = true;
            wrongRing = false;

            // Para o cronômetro se for o último anel
            if (i == NUM_RINGS - 1)
            {
                timerRunning = false;
                lastElapsed = getTime() - startTime; // registra o tempo final
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windW, 0, windH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    if (!collided)
    {
        char buffer[64];
        double currentTime = getTime();
        if (!countdownFinished)
        {
            double remaining = countdownTime - (currentTime - countdownStart);
            if (remaining <= 0.0)
            {
                countdownFinished = true;
                startTime = getTime();
                timerRunning = true;
                canMove = true;
                remaining = 0.0;
            }
            sprintf(buffer, "Comecando em: %.0f", ceil(remaining));
        }
        else
        {
            double elapsed = timerRunning ? (currentTime - startTime) : (lastElapsed);
            lastElapsed = elapsed;
            sprintf(buffer, "Tempo: %.2f s", elapsed);
        }

        int len = strlen(buffer);
        float charWidth = 10.0f;
        float x = 5, y = windH - 30;
        float w = len * charWidth, h = 25;

        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(x + 5, y + 5);
        for (int i = 0; buffer[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    if (collided || lastRingIndex == NUM_RINGS - 1)
    {
        const char *msg = "APERTE R PARA REINICIAR";
        int len = strlen(msg);
        float charWidth = 10.0f;
        float x = (windW - len * charWidth) / 2.0f;
        float y = windH / 2.0f;
        float w = len * charWidth, h = 25;

        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(x + 5, y + 5);
        for (int i = 0; msg[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[i]);
    }

    if (collided && !explosionActive)
    {
        explosionActive = true;
        explosionStartTime = getTime();
        for (int i = 0; i < NUM_PARTICLES; i++)
        {
            particles[i].x = playerX;
            particles[i].y = playerY;
            particles[i].z = playerZ;
            // velocidades aleatórias pequenas
            particles[i].vx = ((rand() % 200) - 100) / 100.0f;
            particles[i].vy = ((rand() % 200) - 100) / 100.0f;
            particles[i].vz = ((rand() % 200) - 100) / 100.0f;
            particles[i].active = true;
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // explosão
    if (explosionActive)
    {
        double t = getTime() - explosionStartTime;
        if (t > 1.0)
        {
            explosionActive = false;
        }
        else
        {
            glDisable(GL_LIGHTING);
            glPointSize(5.0f);
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.5f, 0.0f);
            for (int i = 0; i < NUM_PARTICLES; i++)
            {
                if (!particles[i].active) continue;
                float px = particles[i].x + particles[i].vx * t;
                float py = particles[i].y + particles[i].vy * t;
                float pz = particles[i].z + particles[i].vz * t;
                glVertex3f(px, py, pz);
            }
            glEnd();
            glEnable(GL_LIGHTING);
        }
    }

    glutSwapBuffers();
}

void drawText(float x, float y, const char *text)
{
    glRasterPos2f(x, y); // posição em coordenadas de tela (0..1)
    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
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
        glutPostRedisplay(); // garante que a tela será redesenhada a cada frame assim o relogio funcion
        lastTime = currentTime;
    }
}

void resetGame()
{
    camX = 0;
    camY = 5.0f;
    camZ = 0;
    playerX = 0;
    playerY = 0;
    playerZ = 0;
    alpha = 0.0f, beta = 0.0f, delta = 1.0f; // ângulos de rotação e zoom
    movement = 0.1f;

    lastRingIndex = -1;
    wrongRing = false;
    for (int i = 0; i < NUM_RINGS; i++)
        rings[i].passed = false;

    initBuildings();
    initRings();

    startTime = getTime();
    countdownStart = getTime();
    countdownFinished = false;
    timerRunning = false;
    canMove = false;

    glutPostRedisplay();
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
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
    // programUI();

    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}

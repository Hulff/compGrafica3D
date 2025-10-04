
#include "menu.h"
#include "config.h"
#include "input.h"

#include <GL/freeglut.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SPEED 0.01f
#define NUM_RINGS 10
#define NUM_BUILDINGS 20
#define GROUND_Y    (-4.75f)
#define PLANE_HALF  0.6f

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
float camX = 0, camY = 5.0f, camZ = 0;            // posição da camera
float playerX = 0, playerY = 0, playerZ = 0;   // posição do player
bool timerRunning = false; // inicia como false, só roda depois do countdown
double lastElapsed = 0.0; // guarda o último tempo decorrido quando o cronômetro para
double countdownStart = 0;   // momento em que o countdown começou
double countdownTime = 3.0;  // duração do countdown em segundos
bool countdownFinished = false;
bool canMove = false; // false enquanto o countdown não terminar


float movement = 0.1f; // velocidade de movimento da câmera

int lastRingIndex = -1; // índice do último anel passado

// variaveis de controle de tempo / frame rate
const float fps = 60.0f;
const float frameDelay = 1.0f / fps; // segundos
double lastTime = 0.0;
static double startTime = 0;
bool wrongRing = false;
// TODO adicionar iluminação

// utilziando assimp para importar o modelo 3D
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
    int v[3];  // índices dos vértices
    int vt[3]; // índices das texturas
    int vn[3]; // índices das normais
} Face;

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

    char line[128];

    // Contar primeiro para alocar memória
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
            fCount++;
    }

    model->vertices = malloc(sizeof(Vec3) * vCount);
    model->normals = malloc(sizeof(Vec3) * vnCount);
    model->texcoords = malloc(sizeof(Vec2) * vtCount);
    model->faces = malloc(sizeof(Face) * fCount);

    model->numVertices = vCount;
    model->numNormals = vnCount;
    model->numTexcoords = vtCount;
    model->numFaces = fCount;

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
            Face face;
            int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                 &face.v[0], &face.vt[0], &face.vn[0],
                                 &face.v[1], &face.vt[1], &face.vn[1],
                                 &face.v[2], &face.vt[2], &face.vn[2]);
            if (matches != 9)
            {
                printf("Erro: formato de face não suportado\n");
                fclose(file);
                return 0;
            }
            for (int i = 0; i < 3; i++)
            {
                face.v[i]--; // OBJ indices começam em 1
                face.vt[i]--;
                face.vn[i]--;
            }
            model->faces[fi++] = face;
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
            if (model->numNormals > 0)
                glNormal3f(model->normals[f.vn[j]].x,
                           model->normals[f.vn[j]].y,
                           model->normals[f.vn[j]].z);
            if (model->numTexcoords > 0)
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

// checa interseção de duas AABB
static bool aabb_intersect(float axMin, float axMax, float ayMin, float ayMax, float azMin, float azMax,
                           float bxMin, float bxMax, float byMin, float byMax, float bzMin, float bzMax)
{
    return (axMin <= bxMax && axMax >= bxMin) &&
           (ayMin <= byMax && ayMax >= byMin) &&
           (azMin <= bzMax && azMax >= bzMin);
}

// colisão avião e chão
bool checkCollisionWithGround(float py, float halfPlane)
{
    float planeBottom = py - halfPlane;
    return (planeBottom <= GROUND_Y);
}

// colisão avião e prédio
bool checkCollisionWithBuildingIndex(int i, float px, float py, float pz, float halfPlane)
{
    float width  = 1.0f * 1.2f;            // X (mesma escala do drawScenario)
    float height = 1.0f * scaleFactors[i]; // Y (altura do prédio)
    float depth  = 1.0f * 1.0f;            // Z

    float bx = buildings[i].x;
    float bz = buildings[i].z;

    // Altura real do prédio considerando o chão
    float byMin = GROUND_Y;           // base do prédio
    float byMax = GROUND_Y + height;  // topo do prédio

    float bxMin = bx - width / 2.0f;
    float bxMax = bx + width / 2.0f;
    float bzMin = bz - depth / 2.0f;
    float bzMax = bz + depth / 2.0f;

    // margem de segurança para que só detecte colisão quando estiver realmente próximo
    float collisionBuffer = 0.1f;

    // limites do avião com buffer
    float axMin = px - halfPlane + collisionBuffer;
    float axMax = px + halfPlane - collisionBuffer;
    float ayMin = py - halfPlane + collisionBuffer;
    float ayMax = py + halfPlane - collisionBuffer;
    float azMin = pz - halfPlane + collisionBuffer;
    float azMax = pz + halfPlane - collisionBuffer;

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
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        buildings[i].x = (rand() % 20 - 10) / 2.0f;
        buildings[i].z = -(float)(i + 1) * 7.0f;
        buildings[i].y = 0.0f;

        scaleFactors[i] = 5.0f + (rand() % 11); // fator de escala aleatório entre 2  e 5
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
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-250.0f, 0.0f,  250.0f);
        glTexCoord2f(repeat, 0.0f); glVertex3f( 250.0f, 0.0f,  250.0f);
        glTexCoord2f(repeat, repeat); glVertex3f( 250.0f, 0.0f, -250.0f);
        glTexCoord2f(0.0f, repeat); glVertex3f(-250.0f, 0.0f, -250.0f);
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
//desenha os predios manualmente
void desenhaPredioManual(float size)
{
    float s = size / 2.0f;

    glBegin(GL_QUADS);
    // frente
    glNormal3f(0,0,1);
      glVertex3f(-s,-s, s);
      glVertex3f( s,-s, s);
      glVertex3f( s, s, s);
      glVertex3f(-s, s, s);
    // trás
    glNormal3f(0,0,-1);
      glVertex3f(-s,-s,-s);
      glVertex3f(-s, s,-s);
      glVertex3f( s, s,-s);
      glVertex3f( s,-s,-s);
    // esquerda
    glNormal3f(-1,0,0);
      glVertex3f(-s,-s,-s);
      glVertex3f(-s,-s, s);
      glVertex3f(-s, s, s);
      glVertex3f(-s, s,-s);
    // direita
    glNormal3f(1,0,0);
      glVertex3f( s,-s,-s);
      glVertex3f( s, s,-s);
      glVertex3f( s, s, s);
      glVertex3f( s,-s, s);
    // topo
    glNormal3f(0,1,0);
      glVertex3f(-s, s,-s);
      glVertex3f(-s, s, s);
      glVertex3f( s, s, s);
      glVertex3f( s, s,-s);
    // fundo
    glNormal3f(0,-1,0);
      glVertex3f(-s,-s,-s);
      glVertex3f( s,-s,-s);
      glVertex3f( s,-s, s);
      glVertex3f(-s,-s, s);
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

        glCullFace(GL_BACK);          // cull das faces de trás
        desenhaPredioManual(1.0f);

        glPopMatrix();
    }

    glDisable(GL_CULL_FACE);
}

void drawPlayer()
{
    glPushMatrix();

    float dirX = cosf(alpha) * sinf(beta);
    float dirY = sinf(alpha);
    float dirZ = -cosf(alpha) * cosf(beta);

    float distance = 4.0f; // distância à frente da câmera
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
    if(roll > 30) roll = 30;
    if(roll < -30) roll = -30;
    glRotatef(roll * adjustmentFactor, 0, 0, 1);
    
    

    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(0.3f, 0.3f, 0.3f);

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
    glEnable(GL_LIGHT0);         // ativa a luz 0
    glEnable(GL_COLOR_MATERIAL); // deixa glColor influenciar material
    glEnable(GL_CULL_FACE); // ativa backface culling (desenha só faces visíveis)
    glFrontFace(GL_CCW); // frente = anti-horário (padrão)

    // parâmetros da luz
    GLfloat lightPos[]     = { 0.0f, 10.0f, 5.0f, 1.0f }; // posição (w=1 → pontual)
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[]= { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // material básico (pra objetos brilharem com especular)
    GLfloat mat_specular[]  = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    // === fim iluminação ===

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)windW / windH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    initRings();
    initBuildings();
    // carregar textura do chão (arquivo deve existir)
    groundTexture = loadTexture("textures/grass.jpg"); // coloque sua imagem "grass.jpg" na pasta do executável
    loadOBJ("models/Jet_Lowpoly.obj", &model);         // coloque seu modelo "Jet_Lowpoly.obj" na pasta do executável

    //Countdown
    startTime = getTime();       // marca o início do tempo total
    countdownStart = getTime();  // marca o início do countdown
    countdownFinished = false;

    glCullFace(GL_BACK);

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
    
    GLfloat lightPos[] = { 0.0f, 20.0f, 20.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glPushMatrix();
    drawGround();
    drawScenario();
    drawRings();
    glPopMatrix();
    drawPlayer();

    if (checkCollisionWithGround(playerY, PLANE_HALF)) {
        printf("Colisão com o chão\n");
        printf("playerY=%.2f, bottom=%.2f, ground=%.2f\n", playerY, playerY - PLANE_HALF, GROUND_Y);
        movement = 0.0f;       // trava movimento
        timerRunning = false;  // opcional: para o cronômetro
    }

    for (int i = 0; i < NUM_BUILDINGS; i++) {
        if (checkCollisionWithBuildingIndex(i, playerX, playerY, playerZ, PLANE_HALF)) {
            printf("Colisão com prédio %d\n", i);
            movement = 0.0f;
            timerRunning = false;
            break; // já basta detectar uma
        }
    }

    // check de passagem nos aneis
    for (int i = 0; i < NUM_RINGS; i++)
    {
        float dx = camX - rings[i].x;
        float dy = camY - rings[i].y;
        float dz = camZ - rings[i].z;
        float dist2 = dx * dx + dy * dy + dz * dz;

        float threshold = 2.0f; // distância para considerar que passou pelo anel

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
                printf("Tempo levado: %.2f segundos\n", getTime() - startTime);
                timerRunning = false;
            }
            else
            {
                printf("Aneis restantes: %d\n", NUM_RINGS - i - 1);
                printf("Tempo decorrido: %.2f segundos\n", getTime() - startTime);
            }
        }
    }

    // countdown e cronômetro
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windW, 0, windH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

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
        lastElapsed = elapsed; // salvar último tempo caso cronômetro tenha parado
        sprintf(buffer, "Tempo: %.2f s", elapsed);
    }

    // calcular tamanho do fundo baseado no comprimento do texto
    int len = strlen(buffer);
    float charWidth = 10.0f;
    float x = 5, y = windH - 30;
    float w = len * charWidth, h = 25;

    // fundo preto atrás do texto
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();

    // desenhar texto em branco
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x + 5, y + 5);
    for (int i = 0; buffer[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);



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

        lastTime = currentTime;
    }
}

void resetGame()
{
    camX = 0; camY = 5.0f; camZ = 0;
    playerX = 0; playerY = 0; playerZ = 0;
    movement = 0.1f;

    lastRingIndex = -1;
    wrongRing = false;
    for (int i = 0; i < NUM_RINGS; i++)
        rings[i].passed = false;

    initBuildings();

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

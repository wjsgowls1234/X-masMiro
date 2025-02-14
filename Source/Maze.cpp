#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace std;

const double M_PI = acos(-1);

GLsizei winWidth = 1000, winHeight = 700;

float playerX = -3.3f + 1 * 0.6f, playerY = 0.3f, playerZ = -3.3f + 1 * 0.6f;
float playerYaw = 0.0f;
float cameraSpeed = 0.1f;


int activeObjectCount = 0;

int modelMaze[11][11] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

struct GameObject {
    float x, y, z;
    float vx, vy, vz;
    float ax, ay, az;
    int modelIndex;
    bool active;
};

vector<GameObject> gameObjects;

void InitializeGameObjects() {
    activeObjectCount = 0;

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (modelMaze[i][j] != 0) {
                float x = i * 0.6f - 3.3f;
                float z = j * 0.6f - 3.3f;
                float y = 0.1f;

                GameObject obj = { x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, modelMaze[i][j], true };

                if (modelMaze[i][j] == 3) {
                    obj.vx = ((rand() % 100) / 100.0f - 0.5f) * 0.2f;
                    obj.vz = ((rand() % 100) / 100.0f - 0.5f) * 0.2f;
                    obj.ax = 0.0f;
                    obj.az = 0.0f;
                }

                gameObjects.push_back(obj);
                activeObjectCount++;
            }
        }
    }
}

GLfloat MyVertices[8][3] = {
    {-0.3, -0.3, 0.3},
    {-0.3, 0.3, 0.3},
    {0.3, 0.3, 0.3},
    {0.3, -0.3, 0.3},
    {-0.3, -0.3, -0.3},
    {-0.3, 0.3, -0.3},
    {0.3, 0.3, -0.3},
    {0.3, -0.3, -0.3}
};

GLubyte MyVertexList[24] = {
    0, 3, 2, 1,
    2, 3, 7, 6,
    0, 4, 7, 3,
    1, 2, 6, 5,
    4, 5, 6, 7,
    0, 1, 5, 4
};

int maze[11][11] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
struct Point {
    float x, y, z;
};

struct Face {
    int ip[3];
};

int pnum;
int fnum;
Point* mpoint = NULL;
Face* mface = NULL;

int MyListID;
GLuint mazeTexture;
std::string textureFileName = "treetexture.bmp";
GLuint floorTexture;
std::string floorTextureFileName = "redcarpettexture.bmp";

bool CheckCollision(float newX, float newZ) {
    int mazeX = static_cast<int>(round((newX + 3.3f) / 0.6f));
    int mazeZ = static_cast<int>(round((newZ + 3.3f) / 0.6f));

    if (mazeX < 0 || mazeX >= 11 || mazeZ < 0 || mazeZ >= 11) {
        return true;
    }

    return maze[mazeX][mazeZ] == 1;
}

bool isSuccess = false;
bool showSuccessImage = false;
int successDelay = 2000;
GLuint successTexture;
std::string successImageFileName = "merrychristmas.bmp";

unsigned char* Read_BmpImage(const char name[], int* width, int* height, int* components) {
    FILE* BMPfile;
    errno_t err;
    GLubyte garbage;
    long size;
    int start_point, x;
    GLubyte temp[3];
    GLubyte start[4], w[4], h[4];
    unsigned char* read_image;

    err = fopen_s(&BMPfile, name, "rb");
    if (err != 0 || BMPfile == NULL) {
        cerr << "Error opening file: " << name << endl;
        return nullptr;
    }

    for (x = 0; x < 10; x++) {
        fread(&garbage, 1, 1, BMPfile);
    }

    fread(&start[0], 1, 4, BMPfile);

    for (x = 0; x < 4; x++) {
        fread(&garbage, 1, 1, BMPfile);
    }

    fread(&w[0], 1, 4, BMPfile);
    fread(&h[0], 1, 4, BMPfile);

    (*width) = (w[0] + 256 * w[1] + 256 * 256 * w[2] + 256 * 256 * 256 * w[3]);
    (*height) = (h[0] + 256 * h[1] + 256 * 256 * h[2] + 256 * 256 * 256 * h[3]);
    size = (*width) * (*height);
    start_point = (start[0] + 256 * start[1] + 256 * 256 * start[2] + 256 * 256 * 256 * start[3]);

    read_image = (unsigned char*)malloc(size * 3);

    for (x = 0; x < (start_point - 26); x++) {
        fread(&garbage, 1, 1, BMPfile);
    }

    for (x = 0; x < (size * 3); x = x + 3) {
        fread(&temp[0], 1, 3, BMPfile);
        read_image[x] = temp[2];
        read_image[x + 1] = temp[1];
        read_image[x + 2] = temp[0];
    }
    fclose(BMPfile);
    return read_image;
}

void InitializeTexture(GLuint* texture, const std::string& fileName) {
    unsigned char* imageData;
    int width, height, depth;
    imageData = Read_BmpImage(fileName.c_str(), &width, &height, &depth);

    if (imageData == nullptr) {
        cerr << "Error: Failed to load texture: " << fileName << endl;
        return;
    }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    free(imageData);
}

void InitializeLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos0[] = { 0.0f, 5.0f, 0.0f, 1.0f };
    GLfloat lightDiffuse0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightAmbient0[] = { 0.3f, 0.3f, 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void MakeCubePlayList() {
    MyListID = glGenLists(1);
    glNewList(MyListID, GL_COMPILE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, MyVertices);

    glBindTexture(GL_TEXTURE_2D, mazeTexture);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    for (GLint i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            int index = MyVertexList[i * 4 + j];

            glTexCoord2f((j == 1 || j == 2), (j == 2 || j == 3));
            glVertex3fv(MyVertices[index]);
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEndList();
}

void DrawCube(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glCallList(MyListID);
    glPopMatrix();
}

void DrawFloor(float size, float height) {
    float halfSize = size / 2.0f;

    GLfloat matAmbient[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat matShininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, -height, -halfSize);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(halfSize, -height, -halfSize);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(halfSize, -height, halfSize);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-halfSize, -height, halfSize);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

GLuint LoadTexture(const char* filename) {
    GLuint texture;
    int width, height, channels;
    unsigned char* image = Read_BmpImage(filename, &width, &height, &channels);
    if (!image) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    free(image);
    return texture;
}

GLuint skyboxTexture;

void InitializeSkybox() {
    const char* file = "sky.bmp";

    unsigned char* imageData;
    int width, height, depth;
    imageData = Read_BmpImage(file, &width, &height, &depth);

    if (!imageData) {
        std::cerr << "Failed to load skybox texture." << std::endl;
        return;
    }

    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    free(imageData);
}

void RenderSkybox(float size) {
    float halfSize = size / 2.0f;

    glPushMatrix();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, -halfSize, -halfSize);
    glTexCoord2f(0.25f, 0.0f); glVertex3f(halfSize, -halfSize, -halfSize);
    glTexCoord2f(0.25f, 0.5f); glVertex3f(halfSize, halfSize, -halfSize);
    glTexCoord2f(0.0f, 0.5f); glVertex3f(-halfSize, halfSize, -halfSize);

    glTexCoord2f(0.25f, 0.0f); glVertex3f(halfSize, -halfSize, halfSize);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-halfSize, -halfSize, halfSize);
    glTexCoord2f(0.5f, 0.5f); glVertex3f(-halfSize, halfSize, halfSize);
    glTexCoord2f(0.25f, 0.5f); glVertex3f(halfSize, halfSize, halfSize);

    glTexCoord2f(0.5f, 0.0f); glVertex3f(-halfSize, -halfSize, halfSize);
    glTexCoord2f(0.75f, 0.0f); glVertex3f(-halfSize, -halfSize, -halfSize);
    glTexCoord2f(0.75f, 0.5f); glVertex3f(-halfSize, halfSize, -halfSize);
    glTexCoord2f(0.5f, 0.5f); glVertex3f(-halfSize, halfSize, halfSize);

    glTexCoord2f(0.75f, 0.0f); glVertex3f(halfSize, -halfSize, -halfSize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(halfSize, -halfSize, halfSize);
    glTexCoord2f(1.0f, 0.5f); glVertex3f(halfSize, halfSize, halfSize);
    glTexCoord2f(0.75f, 0.5f); glVertex3f(halfSize, halfSize, -halfSize);

    glTexCoord2f(0.25f, 0.5f); glVertex3f(-halfSize, halfSize, -halfSize);
    glTexCoord2f(0.5f, 0.5f); glVertex3f(halfSize, halfSize, -halfSize);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(halfSize, halfSize, halfSize);
    glTexCoord2f(0.25f, 1.0f); glVertex3f(-halfSize, halfSize, halfSize);

    glTexCoord2f(0.25f, 0.5f); glVertex3f(-halfSize, -halfSize, halfSize);
    glTexCoord2f(0.5f, 0.5f); glVertex3f(halfSize, -halfSize, halfSize);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(halfSize, -halfSize, -halfSize);
    glTexCoord2f(0.25f, 1.0f); glVertex3f(-halfSize, -halfSize, -halfSize);

    glEnd();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}

struct ModelInfo {
    string fileName;
    float scale;
    float color[3];
};

ModelInfo modelInfo[5] = {
    {"", 0.0f, {0.0f, 0.0f, 0.0f}},
    {"tree.dat", 0.0007f, {0.0f, 1.0f, 0.0f}},
    {"bell.dat", 0.0003f, {1.0f, 1.0f, 0.0f}},
    {"ball.dat", 0.0002f, {1.0f, 0.0f, 0.0f}},
    {"snowman.dat", 0.0003f, {1.0f, 1.0f, 1.0f}} 
};

Point cnormal(Point a, Point b, Point c) {
    Point p, q, r;
    double val;
    p.x = a.x - b.x; p.y = a.y - b.y; p.z = a.z - b.z;
    q.x = c.x - b.x; q.y = c.y - b.y; q.z = c.z - b.z;
    r.x = p.y * q.z - p.z * q.y;
    r.y = p.z * q.x - p.x * q.z;
    r.z = p.x * q.y - p.y * q.x;
    val = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
    r.x = r.x / val; r.y = r.y / val; r.z = r.z / val;
    return r;
}

void ReadModel(int modelIndex, float x, float y, float z) {
    if (modelIndex == 0) return;

    ModelInfo& info = modelInfo[modelIndex];
    FILE* f1; char s[81]; int i;
    if ((f1 = fopen(info.fileName.c_str(), "rt")) == NULL) {
        cerr << "Error: Unable to load model file " << info.fileName << endl;
        return;
    }

    fscanf(f1, "%s", s);
    fscanf(f1, "%s", s);
    fscanf(f1, "%d", &pnum);
    mpoint = new Point[pnum];

    for (int i = 0; i < pnum; i++) {
        fscanf(f1, "%f", &mpoint[i].x); fscanf(f1, "%f", &mpoint[i].y); fscanf(f1, "%f", &mpoint[i].z);
    }

    fscanf(f1, "%s", s);
    fscanf(f1, "%s", s);
    fscanf(f1, "%d", &fnum);

    mface = new Face[fnum];
    for (i = 0; i < fnum; i++) {
        fscanf(f1, "%d", &mface[i].ip[0]); fscanf(f1, "%d", &mface[i].ip[1]); fscanf(f1, "%d", &mface[i].ip[2]);
    }
    fclose(f1);

    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT);

    glTranslatef(x, y, z);
    glScalef(info.scale, info.scale, info.scale);
    glColor3f(info.color[0], info.color[1], info.color[2]);
    for (i = 0; i < fnum; i++) {
        Point norm = cnormal(mpoint[mface[i].ip[2]], mpoint[mface[i].ip[1]], mpoint[mface[i].ip[0]]);
        glBegin(GL_TRIANGLES);
        glNormal3f(norm.x, norm.y, norm.z);
        glVertex3f(mpoint[mface[i].ip[0]].x,
            mpoint[mface[i].ip[0]].y, mpoint[mface[i].ip[0]].z);
        glNormal3f(norm.x, norm.y, norm.z);
        glVertex3f(mpoint[mface[i].ip[1]].x,
            mpoint[mface[i].ip[1]].y, mpoint[mface[i].ip[1]].z);
        glNormal3f(norm.x, norm.y, norm.z);
        glVertex3f(mpoint[mface[i].ip[2]].x,
            mpoint[mface[i].ip[2]].y, mpoint[mface[i].ip[2]].z);
        glEnd();
    }

    glPopAttrib();
    glPopMatrix();

    delete[] mpoint;
    delete[] mface;
}

void ShowSuccessImage(int value) {
    showSuccessImage = true;
    glutPostRedisplay();
}

void PlayChristmasSound() {
    PlaySound(TEXT("JingleBells.wav"), NULL, SND_ASYNC | SND_LOOP);
}

void BackgroundSound() {
    PlaySound(TEXT("Christmas Special 4 (Instrumental).wav"), NULL, SND_ASYNC | SND_LOOP);
}

void ItemSound() {
    std::thread([]() {
        PlaySound(TEXT("Item2A.wav"), NULL, SND_SYNC);

        if (activeObjectCount != 0)
        {
            BackgroundSound();
        }
        }).detach();
}

void CheckObjectCollision() {
    for (auto& obj : gameObjects) {
        if (obj.active) {
            float distance = sqrt(pow(playerX - obj.x, 2) + pow(playerZ - obj.z, 2));
            if (distance < 0.3f) {
                obj.active = false;
                activeObjectCount--;
                ItemSound();
                std::cout << "Object deactivated. Remaining active objects: " << activeObjectCount << "\n";

                if (activeObjectCount == 0 && !isSuccess) {
                    isSuccess = true;
                    std::cout << "Success! All objects are deactivated.\n";
                    PlaySound(NULL, NULL, 0);
                    PlayChristmasSound();
                    glutTimerFunc(successDelay, ShowSuccessImage, 0);
                }
            }
        }
    }
}


void RenderQuarterView() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0);

    DrawFloor(10.0f, 0.1f);

    int maze[11][11] = {
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
     {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1},
     {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
     {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };


    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (maze[i][j] == 1) {
                DrawCube(i * 0.6f - 3.3f, 0.0f, j * 0.6f - 3.3f);
            }
        }
    }
}

void RenderFirstPersonView() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lookX = playerX + cos(playerYaw);
    float lookZ = playerZ + sin(playerYaw);

    gluLookAt(playerX, playerY, playerZ, lookX, playerY, lookZ, 0.0f, 1.0f, 0.0f);

    DrawFloor(10.0f, 0.1f);

    int maze[11][11] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (maze[i][j] == 1) {
                DrawCube(i * 0.6f - 3.3f, 0.0f, j * 0.6f - 3.3f);
            }
        }
    }
}

void RenderModels() {
    int modelMaze[11][11] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (modelMaze[i][j] != 0) {

                float x = i * 0.6f - 3.3f;
                float z = j * 0.6f - 3.3f;
                float y = 0.1f;

                ReadModel(modelMaze[i][j], x, y, z);
            }
        }
    }
}

void RenderObjects() {
    for (const auto& obj : gameObjects) {
        if (obj.active) {
            ReadModel(obj.modelIndex, obj.x, obj.y, obj.z);
        }
    }
}

void RenderSuccessImage() {
    if (!isSuccess) return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, successTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}


void InitializeSuccessTexture() {
    unsigned char* imageData;
    int width, height, depth;
    imageData = Read_BmpImage(successImageFileName.c_str(), &width, &height, &depth);

    if (imageData == nullptr) {
        cerr << "Error: Failed to load success image: " << successImageFileName << endl;
        isSuccess = false;
        return;
    }

    glGenTextures(1, &successTexture);
    glBindTexture(GL_TEXTURE_2D, successTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    free(imageData);
}

void DisplayRemainingObjects() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winWidth, 0, winHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0, 0.0, 0.0);

    glRasterPos2i(10, winHeight - 20);
    string text = "Remaining decorations: " + to_string(activeObjectCount) + " / 4";

    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}


void ProcessMouseMotion(int x, int y) {
    float sensitivity = 0.005f;
    playerYaw += (x - winWidth / 2) * sensitivity;
    glutWarpPointer(winWidth / 2, winHeight / 2);
}

void ProcessSpecialKeys(int key, int x, int y) {
    float newX = playerX;
    float newZ = playerZ;

    switch (key) {
    case GLUT_KEY_UP:
        newX += cameraSpeed * cos(playerYaw);
        newZ += cameraSpeed * sin(playerYaw);
        break;
    case GLUT_KEY_DOWN:
        newX -= cameraSpeed * cos(playerYaw);
        newZ -= cameraSpeed * sin(playerYaw);
        break;
    case GLUT_KEY_LEFT:
        playerYaw -= 0.1f;
        glutPostRedisplay();
        return;
    case GLUT_KEY_RIGHT:
        playerYaw += 0.1f;
        glutPostRedisplay();
        return;
    }

    if (!CheckCollision(newX, newZ)) {
        playerX = newX;
        playerZ = newZ;
    }

    CheckObjectCollision();

    glutPostRedisplay();
}

void InitializeTreeLights() {
    glEnable(GL_LIGHT1);
    GLfloat lightColor[] = { 1.0f, 0.5f, 0.0f, 1.0f };
    GLfloat lightPosition[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
}

void MyReshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -500.0, 500.0);
}

void MyDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderSkybox(50.0f);

    if (isSuccess && showSuccessImage) {

        RenderSuccessImage();

    }
    else {
        int width = glutGet(GLUT_WINDOW_WIDTH);
        int height = glutGet(GLUT_WINDOW_HEIGHT);

        int squareSize = (width / 5 < height * 2 / 5) ? width / 5 : height * 2 / 5;
        glViewport(0, height - squareSize, squareSize, squareSize);
        RenderQuarterView();

        glViewport(0, 0, width, height);
        RenderFirstPersonView();

        RenderObjects();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        DisplayRemainingObjects();
        glPopAttrib();
    }

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(270, 45);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Decorate the Christmas Tree !");

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    InitializeTexture(&mazeTexture, textureFileName);
    InitializeTexture(&floorTexture, floorTextureFileName);
    InitializeLighting();
    MakeCubePlayList(); 
    InitializeGameObjects();
    InitializeSuccessTexture();
    InitializeSkybox();

    glutDisplayFunc(MyDisplay);
    glutSpecialFunc(ProcessSpecialKeys);

    BackgroundSound();

    glutMainLoop();
    return 0;
}
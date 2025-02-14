#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <thread>

using namespace std;

const double M_PI = acos(-1);

class xPoint3D
{
public:
    float x, y, z, w;
    xPoint3D() { x = y = z = 0; w = 1; };
    xPoint3D(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
};

vector<xPoint3D> arInputPoints;
vector<xPoint3D> arRotPoints;
vector<vector<int>> arFaces;
GLsizei winWidth = 1000, winHeight = 700;
int pnum = 0;
int fnum = 0;

void init(void)
{
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -500.0, 500.0);
}

void plotPoints(void)
{
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (const auto& pt : arInputPoints)
    {
        glVertex3f(pt.x, pt.y, pt.z);
    }
    for (const auto& pt : arRotPoints)
    {
        glVertex3f(pt.x, pt.y, pt.z);
    }
    glEnd();
}

void generateFaces(int segments)
{
    arFaces.clear();
    int pointsPerRing = segments;
    int totalRings = pnum;

    for (int i = 0; i < totalRings - 1; ++i)
    {
        int baseIndex = i * pointsPerRing;
        int nextBaseIndex = (i + 1) * pointsPerRing;

        for (int j = 0; j < pointsPerRing; ++j)
        {
            int current = baseIndex + j;
            int next = baseIndex + (j + 1) % pointsPerRing;
            int nextRing = nextBaseIndex + j;
            int nextRingNext = nextBaseIndex + (j + 1) % pointsPerRing;

            arFaces.push_back({ current, next, nextRing });
            arFaces.push_back({ next, nextRingNext, nextRing });
        }
    }
}

void xRotation(float fRotAngle)
{
    arRotPoints.clear();

    int segments = static_cast<int>(360.0f / fRotAngle);
    for (const auto& pt : arInputPoints)
    {
        for (float angle = 0.0f; angle < 360.0f; angle += fRotAngle)
        {
            float rad = angle * (M_PI / 180.0f);
            xPoint3D newpt;
            newpt.x = pt.x;
            newpt.y = pt.y * cos(rad) - pt.z * sin(rad);
            newpt.z = pt.y * sin(rad) + pt.z * cos(rad);
            arRotPoints.push_back(newpt);
        }
    }

    pnum = arInputPoints.size();
    generateFaces(segments);
}

void yRotation(float fRotAngle)
{
    arRotPoints.clear();

    int segments = static_cast<int>(360.0f / fRotAngle);
    for (const auto& pt : arInputPoints)
    {
        for (float angle = 0.0f; angle < 360.0f; angle += fRotAngle)
        {
            float rad = angle * (M_PI / 180.0f);
            xPoint3D newpt;
            newpt.x = pt.x * cos(rad) + pt.z * sin(rad);
            newpt.y = pt.y;
            newpt.z = -pt.x * sin(rad) + pt.z * cos(rad);
            arRotPoints.push_back(newpt);
        }
    }

    pnum = arInputPoints.size();
    generateFaces(segments);
}

void saveModel()
{
    FILE* fout = fopen("snowman.dat", "w");

    pnum = static_cast<int>(arInputPoints.size() + arRotPoints.size());

    fprintf(fout, "VERTEX = %d\n", pnum);
    for (const auto& pt : arRotPoints)
    {
        fprintf(fout, "%.6f %.6f %.6f\n", pt.x, pt.y, pt.z);
    }

    fprintf(fout, "FACE = %d\n", static_cast<int>(arFaces.size()));
    for (const auto& face : arFaces)
    {
        fprintf(fout, "%d %d %d\n", face[0], face[1], face[2]);
    }

    fclose(fout);
}

void mouseClick(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
    if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN)
    {
        xPoint3D pt;
        pt.x = xMouse - winWidth / 2;
        pt.y = (winHeight - yMouse) - winHeight / 2;
        pt.z = 0;

        arInputPoints.push_back(pt);
        glutPostRedisplay();
    }
}

void myDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(-winWidth / 2, 0, 0);
    glVertex3f(winWidth / 2, 0, 0);
    glVertex3f(0, -winHeight / 2, 0);
    glVertex3f(0, winHeight / 2, 0);
    glEnd();

    plotPoints();
    glFlush();
}

void myResetMenu(int resetID)
{
    if (resetID == 1)
    {
        if (!arInputPoints.empty())
        {
            arInputPoints.pop_back();
        }
    }
    else if (resetID == 2)
    {
        arInputPoints.clear();
        arRotPoints.clear();
        arFaces.clear();
    }
    glutPostRedisplay();
}

void closeWindow()
{
    int currentWindow = glutGetWindow();
    if (currentWindow != 0)
    {
        glutDestroyWindow(currentWindow);
        glutMainLoopEvent();
    }
}

void myXRotationMenu(int angleID)
{
    if (angleID == 1)
        xRotation(10.0f);
    else if (angleID == 2)
        xRotation(20.0f);
    else if (angleID == 3)
        xRotation(30.0f);

    saveModel();
    glutPostRedisplay();

    this_thread::sleep_for(chrono::milliseconds(1000));
    glutMainLoopEvent();
    
    closeWindow();

    glutLeaveMainLoop();

    system("CG_FinalProject2.exe");
    exit(0);
}

void myYRotationMenu(int angleID)
{
    if (angleID == 1)
        yRotation(10.0f);
    else if (angleID == 2)
        yRotation(20.0f);
    else if (angleID == 3)
        yRotation(30.0f);

    saveModel();
    glutPostRedisplay();

    this_thread::sleep_for(chrono::milliseconds(1000));
    glutMainLoopEvent();
    
    closeWindow();

    glutLeaveMainLoop();

    system("CG_FinalProject2.exe");
    exit(0);
}

void MyReshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -500.0, 500.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(270, 45);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Click to Draw and Rotate Points");

    init();
    glEnable(GL_DEPTH_TEST);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    GLint MyResetMenuID = glutCreateMenu(myResetMenu);
    glutAddMenuEntry("[1] Clear Last Point", 1);
    glutAddMenuEntry("[2] Reset", 2);

    GLint MyYRotationMenuID = glutCreateMenu(myYRotationMenu);
    glutAddMenuEntry("[1] 10 degrees", 1);
    glutAddMenuEntry("[2] 20 degrees", 2);
    glutAddMenuEntry("[3] 30 degrees", 3);

    GLint MyXRotationMenuID = glutCreateMenu(myXRotationMenu);
    glutAddMenuEntry("[1] 10 degrees", 1);
    glutAddMenuEntry("[2] 20 degrees", 2);
    glutAddMenuEntry("[3] 30 degrees", 3);

    GLint MyMainMenuID = glutCreateMenu([](int id) {});
    glutAddSubMenu("[1] Y-Rotate", MyYRotationMenuID);
    glutAddSubMenu("[2] X-Rotate", MyXRotationMenuID);
    glutAddSubMenu("[3] Clear", MyResetMenuID);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(myDisplay);
    glutMouseFunc(mouseClick);
    glutReshapeFunc(MyReshape);

    glutMainLoop();
    return 0;
}
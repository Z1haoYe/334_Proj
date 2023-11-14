#include <windows.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#include "perlin.hpp"
#include "util.hpp"

#define M_PI 3.14159265358979323846

const int WIDTH = 600;
const int HEIGHT = 600;
int sealevel = 1;
bool shiftPressed = false;
bool is_rotating = false;
float rotation_angle = 0.0f;

int mesh = GL_TRIANGLE_STRIP;
//GL_POINTS
//GL_TRIANGLE_STRIP
//GL_QUADS

float final_terrain[HEIGHT][WIDTH];

float low, high;
bool water = 1;

//default colors
float colorLow[3] = { 0.0f, 0.0f, 1.0f }; // blue
float colorMid[3] = { 0.0f, 1.0f, 0.0f }; // green
float colorHigh[3] = { 1.0f, 1.0f, 1.0f }; // white

float toLight[3] = { 0.0f, 1.0f, 0.0f };

float rotationX = 0, rotationY = 0, lastX = 0, lastY = 0;
float angle = 0.0f;
float axisX = 0.0f;
float axisY = 1.0f;
float axisZ = 0.0f;
int mouseX, mouseY;
int prevMouseX, prevMouseY;
float centerX = WIDTH / 2.0f;
float centerZ = HEIGHT / 2.0f;
int zoom = 1000;

int max_scale;
float terrainScale = 1;

float** generateTerrain(float freq, float amp) {
    init_permutation();

    const float frequency = freq;
    const float amplitude = amp;

    float** terrain = new float* [HEIGHT];
    for (int i = 0; i < HEIGHT; i++) {
        terrain[i] = new float[WIDTH];
    }

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float noise = 0.0f;
            float persistence = 0.5f;
            float scale = 1.0f;
            for (int i = 0; i < 4; i++) {
                float nx = x * frequency * scale;
                float ny = y * frequency * scale;
                float octave = perlin_noise_2d(nx, ny) * 2.0f - 1.0f;
                noise += octave * amplitude * persistence;
                persistence *= 0.5f;
                scale *= 2.0f;
            }
            terrain[y][x] = noise;
        }
    }
    return terrain;
}

void renderTerrain(float** terrain)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(mesh);
    for (int y = 0; y < HEIGHT - 1; y++) {
        for (int x = 0; x < WIDTH; x++) {
            glColor3f(terrain[y][x], terrain[y][x], terrain[y][x]);
            glVertex3f(x, y, terrain[y][x]);
            glColor3f(terrain[y + 1][x], terrain[y + 1][x], terrain[y + 1][x]);
            glVertex3f(x, y + 1, terrain[y + 1][x]);
        }
    }
    glEnd();

    glutSwapBuffers();
}

void init(void)
{
    // set clear color to sky blue
    glClearColor(0.529f, 0.808f, 0.922f, 0.0f);

    // enable depth test
    glEnable(GL_DEPTH_TEST);
}

// function to draw terrain
void drawTerrain(float terrain[HEIGHT][WIDTH], int max_scale, float lowHeight, float highHeight)
{
    float colorFactor;

    // set terrain surface normals
    //glNormal3f(0.0f, 1.0f, 0.0f);    
    // set terrain vertices and texture coordinates
    for (int z = 0; z < HEIGHT - 1; z++)
    {
        glBegin(mesh);

        for (int x = 0; x < WIDTH; x++)
        {
            float height = terrain[z][x];
            float norm_height = height / max_scale;

            // interpolate between low and mid colors
            float r, g, b;

            for (int f = 0; f < 2; f++)
            {
                height = terrain[z + f][x];

                // interpolate between low and mid colors

                if (norm_height < lowHeight) {
                    if (water) colorFactor = norm_height / lowHeight / 2.5;
                    else colorFactor = norm_height / lowHeight;
                }
                else if (norm_height >= highHeight) {
                    colorFactor = (norm_height - lowHeight) / (highHeight - lowHeight);
                }
                else {
                    colorFactor = 1.0;
                }

                r = (1.0 - colorFactor / 1) * colorLow[0] + colorFactor / 1 * colorMid[0];
                g = (1.0 - colorFactor / 1) * colorLow[1] + colorFactor / 1 * colorMid[1];
                b = (1.0 - colorFactor / 1) * colorLow[2] + colorFactor / 1 * colorMid[2];

                // interpolate between mid and high colors
                if (norm_height >= highHeight) {
                    colorFactor = (norm_height - highHeight) / (1.0 - highHeight);
                    r = (1.0 - colorFactor) * colorMid[0] + colorFactor * colorHigh[0];
                    g = (1.0 - colorFactor) * colorMid[1] + colorFactor * colorHigh[1];
                    b = (1.0 - colorFactor) * colorMid[2] + colorFactor * colorHigh[2];
                }

                // set color
                glColor3f(r, g, b);
                //float x1 = terrain[z + 1][x] - terrain[z][x];
                //float x2 = terrain[z][x + 1] - terrain[z][x];
                //float y1 = 1.0f;
                //float y2 = 1.0f;
                //float z1 = z + 1 - z;
                //float z2 = x + 1 - x;
                //float normalX = y1 * z2 - y2 * z1;
                //float normalY = z1 * x2 - z2 * x1;
                //float normalZ = x1 * y2 - x2 * y1;

                //// normalize the surface normal vector
                //float length = sqrtf(normalX * normalX + normalY * normalY + normalZ * normalZ);
                //normalX /= length;
                //normalY /= length;
                //normalZ /= length;

                //float dotProduct = normalX * toLight[0] + normalY * toLight[1] + normalZ * toLight[2];

                //// apply shadow factor based on dot product
                //float shadowFactor = (dotProduct < 0.0f) ? 0.5f : 1.0f;

                //// set color with shadow factor applied
                //glColor3f(r * shadowFactor, g * shadowFactor, b * shadowFactor);

                // set vertex position and texture coordinate
                if (sealevel && water && norm_height <= lowHeight) glVertex3f(x, (lowHeight) * max_scale, z + f);
                else glVertex3f(x, height, z + f);
            }

        }
        glEnd();
    }
}

void update_max_scale(int k)
{
    max_scale = k;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(WIDTH / 2, 500, zoom, WIDTH / 2, 0, HEIGHT / 2, 0, 1, 0);

    glPushMatrix();
    glTranslatef(WIDTH / 2.0f, 0.0f, HEIGHT / 2.0f);
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glTranslatef(-WIDTH / 2.0f, 0.0f, -HEIGHT / 2.0f);


    // Compute X-axis rotation matrix
    float rotMatX[16];
    glPushMatrix();
    glLoadIdentity();
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, rotMatX);
    glPopMatrix();

    // Compute Y-axis rotation matrix
    float rotMatY[16];
    glPushMatrix();
    glLoadIdentity();
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, rotMatY);
    glPopMatrix();

    // Compute view matrix
    float viewMat[16];
    glPushMatrix();
    glMultMatrixf(rotMatY);
    glMultMatrixf(rotMatX);
    glGetFloatv(GL_MODELVIEW_MATRIX, viewMat);
    glPopMatrix();

    // Compute inverse of view matrix
    double inverseViewMat[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, inverseViewMat);
    InvertMatrix(inverseViewMat, inverseViewMat);

    // Compute rotated light direction
    // assuming light points along positive Z-axis
    float rotatedLightDir[4];
    for (int i = 0; i < 4; i++) {
        rotatedLightDir[i] = 0.0f;
        for (int j = 0; j < 4; j++) {
            rotatedLightDir[i] += inverseViewMat[4 * j + i] * toLight[j];
        }
    }


    toLight[0] = rotatedLightDir[0];
    toLight[1] = rotatedLightDir[1];
    toLight[2] = rotatedLightDir[2];
    float magnitude = sqrt(toLight[0] * toLight[0] + toLight[1] * toLight[1] + toLight[2] * toLight[2]);
    toLight[0] /= magnitude;
    toLight[1] /= magnitude;
    toLight[2] /= magnitude;
    //std::cout << toLight[0] << std::endl;
    if (is_rotating) {
        //glPushMatrix();
        glTranslatef(WIDTH / 2.0f, 0.0f, HEIGHT / 2.0f);
        glRotatef(rotation_angle, 0.0f, 1.0f, 0.0f);
        glTranslatef(-WIDTH / 2.0f, 0.0f, -HEIGHT / 2.0f);
        //glPopMatrix();
    }
    drawTerrain(final_terrain, max_scale, low, high);
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    // set viewport to full window size
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // set projection matrix to perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 2000.0);
}

// global variables to store the mouse position


void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        //if (shiftPressed)
        //{
        //    mouseX = x;
        //    mouseY = y;
        //    GLint viewport[4];
        //    glGetIntegerv(GL_VIEWPORT, viewport);

        //    GLdouble viewMat[16];
        //    GLdouble projMat[16];

        //    glGetDoublev(GL_MODELVIEW_MATRIX, viewMat);
        //    glGetDoublev(GL_PROJECTION_MATRIX, projMat);

        //    GLdouble winX = (double)x;
        //    GLdouble winY = (double)viewport[3] - (double)y;
        //    GLdouble winZ = 0.0;
        //    GLdouble posX, posY, posZ;
        //    gluUnProject(winX, winY, winZ, viewMat, projMat, viewport, &posX, &posY, &posZ);

        //    // Map world space point to terrain 2D array index
        //    float terrainX = ((posX + 0.5f) * WIDTH);
        //    float terrainY = ((posZ + 0.5f) * HEIGHT);

        //    int terrainIndexX = (int)terrainX * terrainScale;
        //    int terrainIndexY = (int)terrainY * terrainScale;
        //    //std::cout << terrainIndexX << std::endl;
        //    //std::cout << x << std::endl;
        //    std::cout << terrainScale << std::endl;
        //    if (terrainIndexX > 599) terrainIndexX = 599;
        //    if (terrainIndexY > 599) terrainIndexY = 599;
        //    // Clamp index to valid range
        //    final_terrain[terrainIndexX][terrainIndexY] *= 10;

        //    glutPostRedisplay();
        //}
        mouseX = x;
        mouseY = y;
        
    }

}

void onMotion(int x, int y) {
    float deltaX = (x - mouseX) / 10.0f;
    float deltaY = (y - mouseY) / 10.0f;
    rotationX += deltaY;
    rotationY += deltaX;
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}

void onScroll(int wheel, int direction, int x, int y)
{
    if (direction == 1)
    {
        // zoom in
        zoom -= 10.0f;
    }
    else if (direction == -1)
    {
        // zoom out
        zoom += 10.0f;
    }

    glutPostRedisplay();
}

void rotateScene()
{
    rotation_angle += 0.3f; // Increase rotation angle
    glutPostRedisplay(); // Mark the window as needing to be redisplayed
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 's':
        sealevel = 1 - sealevel;
        break;
    case 'm':
        if (mesh == GL_POINTS) mesh = GL_TRIANGLE_STRIP;
        else if (mesh == GL_TRIANGLE_STRIP) mesh = GL_QUADS;
        else if (mesh == GL_QUADS) mesh = GL_POINTS;
        break;
    case 'a': // Shift key
        shiftPressed = true;
        break;
    case 'r':
        is_rotating = !is_rotating; // Toggle rotation on/off
        if (is_rotating) {
            glutIdleFunc(rotateScene); // Register idle function to rotate scene
        }
        else {
            glutIdleFunc(NULL); // Unregister idle function to stop rotation
        }
        break;
    }
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'a': // Shift key
        shiftPressed = false;
        break;
        // Other cases
    }
}

int main(int argc, char** argv)
{
    float terrain1freq, terrain2freq;
    int terrain1scale, terrain2scale;
    
    std::cout << "Press S to toggle sea level\nPress M to switch mesh\nPress R to toggle rotation\nUse wheel to zoom in/out\n\n" << std::endl;
    //terrain1scale = 300;
    //terrain2scale = 25;
    //terrain1freq = 0.0007f;
    //terrain2freq = 0.008f;
    //low = 0.2;
    //high = 0.8;
    //water = 1;
    //// blue
    //colorLow[0] = 0.0f;
    //colorLow[1] = 0.0f;
    //colorLow[2] = 1.0f;
    //// yellow
    //colorMid[0] = 0.7608f;
    //colorMid[1] = 0.6980f;
    //colorMid[2] = 0.5020f;
    //// green
    //colorHigh[0] = 0.0f;
    //colorHigh[1] = 1.0f;
    //colorHigh[2] = 0.0f;
    

    if (argc == 1)
    {
        char input;


        std::cout << "Enter a character: " << std::endl;
        std::cout << "'m' for mountain\n's' for seaside\n'r' for rivers and lakes\n'p' for plain\n'c' for coastline\nothers for default" << std::endl;
        std::cin >> input;
        switch (input) {
        case 'm':
            terrain1scale = 400;
            terrain2scale = 150;
            terrain1freq = 0.003f;
            terrain2freq = 0.007f;
            low = 0.3;
            high = 0.4;
            water = 0;
            // blue
            colorLow[0] = 0.0f;
            colorLow[1] = 1.0f;
            colorLow[2] = 0.0f;
            // rock
            colorMid[0] = 192.0f / 255;
            colorMid[1] = 165.0f / 255;
            colorMid[2] = 142.0f / 255;
            // green
            colorHigh[0] = 1.0f;
            colorHigh[1] = 1.0f;
            colorHigh[2] = 1.0f;
            break;
        case 's':
            terrain1scale = 40;
            terrain2scale = 5;
            terrain1freq = 0.001f;
            terrain2freq = 0.006f;
            low = 0.3;
            high = 0.6;
            water = 1;
            // blue
            colorLow[0] = 0.0f;
            colorLow[1] = 0.0f;
            colorLow[2] = 1.0f;
            // rock
            colorMid[0] = 192.0f / 255;
            colorMid[1] = 165.0f / 255;
            colorMid[2] = 142.0f / 255;
            // green
            colorHigh[0] = 255.0f / 255;
            colorHigh[1] = 232.0f / 255;
            colorHigh[2] = 124.0f / 255;
            break;
        case 'r':
            terrain1scale = 100;
            terrain2scale = 20;
            terrain1freq = 0.004f;
            terrain2freq = 0.007f;
            low = 0.55;
            high = 0.57;
            water = 1;
            // blue
            colorLow[0] = 14.0f / 255;
            colorLow[1] = 104.0f / 255;
            colorLow[2] = 255.0f / 255;
            // rock
            colorMid[0] = 192.0f / 255;
            colorMid[1] = 165.0f / 255;
            colorMid[2] = 142.0f / 255;
            // green
            colorHigh[0] = 0.0f / 255;
            colorHigh[1] = 232.0f / 255;
            colorHigh[2] = 0.0f / 255;
            break;
        case 'p':
            terrain1scale = 50;
            terrain2scale = 20;
            terrain1freq = 0.002f;
            terrain2freq = 0.01f;
            low = 0.45;
            high = 0.47;
            water = 0;
            // blue
            colorLow[0] = 178.0f / 255;
            colorLow[1] = 154.0f / 255;
            colorLow[2] = 122.0f / 255;
            // rock
            colorMid[0] = 192.0f / 255;
            colorMid[1] = 165.0f / 255;
            colorMid[2] = 142.0f / 255;
            // green
            colorHigh[0] = 178.0f / 255;
            colorHigh[1] = 155.0f / 255;
            colorHigh[2] = 111.0f / 255;
            break;
        case 'c':
            terrain1scale = 300;
            terrain2scale = 25;
            terrain1freq = 0.0007f;
            terrain2freq = 0.008f;
            low = 0.2;
            high = 0.8;
            water = 1;
            // blue
            colorLow[0] = 0.0f;
            colorLow[1] = 0.0f;
            colorLow[2] = 1.0f;
            // yellow
            colorMid[0] = 0.7608f;
            colorMid[1] = 0.6980f;
            colorMid[2] = 0.5020f;
            // green
            colorHigh[0] = 0.0f;
            colorHigh[1] = 1.0f;
            colorHigh[2] = 0.0f;
            break;
        default:
            terrain1scale = 200;
            terrain2scale = 0;
            terrain1freq = 0.005f;
            terrain2freq = 0.005f;
            low = 0.33;
            high = 0.67;
            water = 0;
            // r
            colorLow[0] = 1.0f;
            colorLow[1] = 0.0f;
            colorLow[2] = 0.0f;
            // g
            colorMid[0] = 0.0f;
            colorMid[1] = 1.0f;
            colorMid[2] = 0.0f;
            // b
            colorHigh[0] = 0.0f;
            colorHigh[1] = 0.0f;
            colorHigh[2] = 1.0f;
            break;
        }
    }
    else
    {
        std::ifstream infile(argv[1]); // Open the file for input
        std::string line;
        float myArray[16];
        int i = 0;

        if (infile.is_open()) {
            while (std::getline(infile, line) && i < 16) { // Read one line at a time
                std::stringstream ss(line);
                std::string token;
                while (std::getline(ss, token, ',')) { // Split the line into tokens using commas as separators
                    myArray[i] = std::stof(token); // Convert the token to a float and store it in the array
                    i++;
                }
            }
            infile.close(); // Close the file
        }
        else {
            std::cout << "Unable to open file" << std::endl;
            return 1;
        }

        // Print the values stored in the array
        for (int j = 0; j < 16; j++) {
            //std::cout << "myArray[" << j << "] = " << myArray[j] << std::endl;
            terrain1scale = myArray[0];
            terrain2scale = myArray[1];
            terrain1freq = myArray[2];
            terrain2freq = myArray[3];
            low = myArray[4];
            high = myArray[5];
            water = myArray[6];
            // r
            colorLow[0] = myArray[7];
            colorLow[1] = myArray[8];
            colorLow[2] = myArray[9];
            // g
            colorMid[0] = myArray[10];
            colorMid[1] = myArray[11];
            colorMid[2] = myArray[12];
            // b
            colorHigh[0] = myArray[13];
            colorHigh[1] = myArray[14];
            colorHigh[2] = myArray[15];
        }
    }

    update_max_scale(terrain1scale);
    //overall pattern
    float** terrain1 = generateTerrain(terrain1freq, 200.0f);
    subtract_min(terrain1, HEIGHT, WIDTH);
    norm(terrain1, HEIGHT, WIDTH);
    scale(terrain1, HEIGHT, WIDTH, terrain1scale);

    //detail
    float** terrain2 = generateTerrain(terrain2freq, 200.0f);
    subtract_min(terrain2, HEIGHT, WIDTH);
    norm(terrain2, HEIGHT, WIDTH);
    scale(terrain2, HEIGHT, WIDTH, terrain2scale);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            final_terrain[y][x] = terrain1[y][x] + terrain2[y][x];
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Terrain");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMouseFunc(onMouse);
    glutMotionFunc(onMotion);
    glutMouseWheelFunc(onScroll);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);

    init();

    glutMainLoop();

    return 0;
}

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "vec.h"
#include <time.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600

#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532
// game states
#define INITIAL 0
#define MAIN1 1
#define MAIN2 2

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false, spacebar = false;
int  winWidth, winHeight; 
int gameState = INITIAL;
int xgl, ygl;
double slope;
bool clicked = false;
double frame; // object
double deltax, deltay; // object
double x, y;  // object last position;



typedef struct {
    float r, g, b;
} color_t;

typedef struct {
    vec_t   pos;
    color_t color;
    vec_t   vel;
    double angle;
} sun_t;

typedef struct {
    vec_t   pos;
    color_t color;
    vec_t   vel;
    double angle;
} moon_t;

typedef struct {
    vec_t pos;
    vec_t N;
} vertex_t;

typedef struct {
    vec_t pos;
    bool active;
    vec_t vel;  //velocity
   
} obj_t;

#define NUM 3

sun_t sun = { { -550, -350 }, {1, 1, 1 }, { 1,  1} };
moon_t moon = { { -550, -350 }, {1, 1, 1 }, { 1,  1} };
obj_t  obj  = { { -600, -400 }, true};
vertex_t mouse;

void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

color_t mulColor(float k, color_t c) {
    color_t tmp = { k * c.r, k * c.g, k * c.b };
    return tmp;
}

color_t addColor(color_t c1, color_t c2) {
    color_t tmp = { c1.r + c2.r, c1.g + c2.g, c1.b + c2.b };
    return tmp;
}


double distanceImpact(double d) {
    return (-1.0 / 400.0) * d + 1.0;
}

color_t calculateColor(sun_t source, vertex_t v) {
    vec_t L = subV(source.pos, v.pos);
    vec_t uL = unitV(L);
    float factor = dotP(uL, v.N) * distanceImpact(magV(L));
    return mulColor(factor, source.color);
}

bool testCollision_Obj(sun_t s, obj_t obj) {

    float dx = obj.pos.x - s.pos.x;
    float dy = obj.pos.y - s.pos.y;
    float d = sqrt(dx * dx + dy * dy);

    if (d > 20) {
        return false;
    }
    return
        true;
}

bool testCollision_Obj1Stage(sun_t s, obj_t obj) {  // to make object red first which is the first stage of collision

    float dx = obj.pos.x - s.pos.x;
    float dy = obj.pos.y - s.pos.y;
    float d = sqrt(dx * dx + dy * dy);

    if (d > 40) {
        return false;
    }
    return
        true;
}

// same function for moon
color_t calculateColor(moon_t source, vertex_t v) {
    vec_t L = subV(source.pos, v.pos);
    vec_t uL = unitV(L);
    float factor = dotP(uL, v.N) * distanceImpact(magV(L));
    return mulColor(factor, source.color);
}

bool testCollision_Obj(moon_t m, obj_t obj) {

    float dx = obj.pos.x - m.pos.x;
    float dy = obj.pos.y - m.pos.y;
    float d = sqrt(dx * dx + dy * dy);

    if (d > 20) {
        return false;
    }
    return
        true;
}

bool testCollision_Obj1Stage(moon_t m, obj_t obj) {

    float dx = obj.pos.x - m.pos.x;
    float dy = obj.pos.y - m.pos.y;
    float d = sqrt(dx * dx + dy * dy);

    if (d > 40) {
        return false;
    }
    return
        true;
}

void display_initial() {
    glClearColor(0, 0, 0, 0);
    glColor3ub(126, 242, 215);
    vprint(-200, 100, GLUT_BITMAP_TIMES_ROMAN_24, "CTIS_164");
    vprint(-200, 70, GLUT_BITMAP_TIMES_ROMAN_24, "Homework_4");
    vprint(-200, 40, GLUT_BITMAP_9_BY_15, "Please Press 'Spacebar' to Start");
    vprint(100, 20, GLUT_BITMAP_8_BY_13, "Created by:");
    vprint(100, 5, GLUT_BITMAP_8_BY_13, "Nurjemal Saryyeva (21901138)");

}


void draw_obj() {  // object for day mode

    if (obj.active == true) {
        frame = magV(obj.vel); // like a speed
        //printf("%0.2f\n", frame);
        deltax = cos(sun.angle) * frame;  // for rotation
        deltay = sin(sun.angle) * frame;
        x = obj.pos.x + deltax;
        y = obj.pos.y + deltay;


        glColor3ub(209, 31, 73);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 10);
        glVertex2f(x + 20, y);
        glVertex2f(x, y - 10);
        glEnd();

        glColor3ub(161, 148, 151);
        glRectf(x - 16, y - 5, x, y + 5);

        glColor3ub(227, 220, 222);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x - 16, y + 5);
        glVertex2f(x, y + 5);
        glVertex2f(x, y - 5);
        glVertex2f(x - 16, y - 5);
        glEnd();

        glColor3ub(105, 99, 101);
        glRectf(x - 24, y - 3, x - 16, y + 3);
    }


    if (testCollision_Obj1Stage(sun, obj)) {
        glColor3ub(224, 13, 42);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 10);
        glVertex2f(x + 20, y);
        glVertex2f(x, y - 10);
        glEnd();

        glColor3ub(224, 13, 42);
        glRectf(x - 16, y - 5, x, y + 5);

        glColor3ub(224, 13, 42);
        glRectf(x - 24, y - 3, x - 16, y + 3);
      
    }

    if (testCollision_Obj(sun, obj)) {
        obj.active = false;
    }
   
}

void draw_objfornight() {  // object for night since function used inside this functions has moon_t arguments different

    if (obj.active == true) {
        frame = magV(obj.vel); // like a speed
        //printf("%0.2f\n", frame);
        deltax = cos(moon.angle) * frame;
        deltay = sin(moon.angle) * frame;
        x = obj.pos.x + deltax;
        y = obj.pos.y + deltay;


        glColor3ub(209, 31, 73);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 10);
        glVertex2f(x + 20, y);
        glVertex2f(x, y - 10);
        glEnd();

        glColor3ub(161, 148, 151);
        glRectf(x - 16, y - 5, x, y + 5);

        glColor3ub(227, 220, 222);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x - 16, y + 5);
        glVertex2f(x, y + 5);
        glVertex2f(x, y - 5);
        glVertex2f(x - 16, y - 5);
        glEnd();

        glColor3ub(105, 99, 101);
        glRectf(x - 24, y - 3, x - 16, y + 3);
    }


    if (testCollision_Obj1Stage(moon, obj)) {

        glColor3ub(224, 13, 42);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 10);
        glVertex2f(x + 20, y);
        glVertex2f(x, y - 10);
        glEnd();

        glColor3ub(224, 13, 42);
        glRectf(x - 16, y - 5, x, y + 5);

        glColor3ub(224, 13, 42);
        glRectf(x - 24, y - 3, x - 16, y + 3);

    }

    if (testCollision_Obj(moon, obj)) {
        obj.active = false;
    }

}

void clouds() { // for day mode 
    glColor3ub(252, 252, 252);
    circle(-350, 230, 20);
    circle(-315, 230, 20);
    circle(-320, 270, 35);
    circle(-265, 260, 45);
    circle(-210, 250, 34);

    glColor3ub(252, 252, 252);
    circle(-50, 240, 20);
    circle(-15, 240, 20);
    circle(-20, 280, 35);
    circle(35, 270, 45);
    circle(90, 280, 34);

    glColor3ub(252, 252, 252);
    circle(450, 210, 20);
    circle(485, 210, 20);
    circle(480, 250, 35);
    circle(535, 240, 45);
    circle(590, 240, 34);
}


void display_main_day() {
    //sky
    glColor3ub(72, 168, 224);
    glBegin(GL_QUADS);
    glVertex2f(-500, -300);
    glVertex2f(-500, 300);
    glVertex2f(500, 300);
    glVertex2f(500, -300);
    glEnd();
    clouds();

    //color of the sun
    if (sun.pos.x > 321 ) {
        sun.color.r = 1;
        sun.color.g = 0.7;
        sun.color.b = 0.2;
    }
    else {
        sun.color.r = 1;
        sun.color.g = 0.9;
        sun.color.b = 0.3;
    }
    glColor3f(sun.color.r, sun.color.g, sun.color.b);
    circle(sun.pos.x, sun.pos.y, 40);

    if (clicked == true) {
        draw_obj();
    }

    // ground with slope
    for (x = -500; x <= 500; x++) {

        slope = (-200.0 + 270.0) / (500.0 + 500.0);
        y = slope * (x + 500) - 250;

        vertex_t P = { { x, y}, { 0, 1 } };

        color_t res = calculateColor(sun, P);
        glBegin(GL_LINES);
        glColor3f(res.r, res.g, res.b);
        glVertex2f(x, y);

        glColor3f(0, 0, 0);
        glVertex2f(x, -300);
        glEnd();

    }
}

void display_main_night() {
    //sky
    glColor3ub(38, 36, 115);
    glBegin(GL_QUADS);
    glVertex2f(-500, -300);
    glVertex2f(-500, 300);
    glVertex2f(500, 300);
    glVertex2f(500, -300);
    glEnd();

    //randomly generated stars
    glColor3ub(242, 179, 230);
    glPointSize(3);
    glBegin(GL_POINTS);
    for (int count = 0; count <= 80; count++) {
        int p1 = rand() % 1000 - 499;
        int p2 = rand() % 800 - 399;
        glVertex2f(p1, p2);
    }
    srand(time(NULL));
    glEnd();

    //moon
    glColor3f(moon.color.r, moon.color.g, moon.color.b);
    circle(moon.pos.x, moon.pos.y, 40);
    glColor3ub(38, 36, 115);
    circle(moon.pos.x+35, moon.pos.y+20, 40);

    if (clicked == true) {
        draw_objfornight();
    }

    // ground 
    for (x = -500; x <= 500; x++) {

        slope = (-200.0 + 270.0) / (500.0 + 500.0);
        y = slope * (x + 500) - 250;

        vertex_t P = { { x, y}, { 0, 1 } };

        color_t res = calculateColor(moon, P);
        glBegin(GL_LINES);
        glColor3f(res.r, res.g, res.b);
        glVertex2f(x, y);

        glColor3f(0, 0, 0);
        glVertex2f(x, -300);
        glEnd();

    }
}

void display() {
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (gameState) {
    case INITIAL: display_initial();
        glClearColor(0, 0, 0, 0);
        break;
    case MAIN1: display_main_day();
        break;
    case MAIN2: display_main_night();
        break;
    }


    glutSwapBuffers();
}

void onKeyDown(unsigned char key, int x, int y)
{
    if (key == 27)
        exit(0);
    if (key == ' ' )
        gameState = 1;
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{

    if (key == 27)
        exit(0);
    glutPostRedisplay();
}

void onSpecialKeyDown(int key, int x, int y)
{
   
    switch (key) {
    case GLUT_KEY_UP: up = true; break;
    case GLUT_KEY_DOWN: down = true; break;
    case GLUT_KEY_LEFT: left = true; break;
    case GLUT_KEY_RIGHT: right = true; break;
    }
    glutPostRedisplay();
}


void onSpecialKeyUp(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP: up = false; break;
    case GLUT_KEY_DOWN: down = false; break;
    case GLUT_KEY_LEFT: left = false; break;
    case GLUT_KEY_RIGHT: right = false; break;
    }
    glutPostRedisplay();
}

void onClick(int button, int stat, int x, int y)
{
    // once mouse is clicked in day and night mode chasing object will display 
    if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
        if (gameState == MAIN1 || gameState == MAIN2) {
            obj.pos = mouse.pos;
            clicked = true;
        }
    }

    glutPostRedisplay();
}

// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.

void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
   
    glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
    // Write your codes here.

    mouse.pos.x = x - winWidth / 2;
    mouse.pos.y = winHeight / 2 - y;


    // to refresh the window it calls display() function
    glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    if (gameState == 1) {
        sun.pos = addV(sun.pos, sun.vel);
        if (sun.pos.y > 30) {  // sun rise stage 
            sun.vel.x = 1;
            sun.vel.y = 0;
            sun.angle = angleV(sun.pos);
            //printf("%0.f\n", sun.angle);
        }
        if (sun.pos.x > 150) { // afternoon stage which is stable 
            sun.vel.y = -1;
            sun.angle = angleV(sun.pos);
            //printf("%0.f\n", sun.angle);
        }
        if (sun.pos.x > 500) { // sunset stage 
            sun.pos.x = -600;
            sun.pos.y = -400;
            sun.vel.x = 1;
            sun.vel.y = 1;
            sun.angle = angleV(sun.pos);
            clicked = false;
            gameState = MAIN2;   // after sunset go to the night stage
            //printf("%0.f\n", sun.angle);
        }

        vec_t V = { 0, 1 };
        vec_t W = unitV(subV(obj.pos, sun.pos));


        obj.vel = mulV(2, unitV(subV(sun.pos, obj.pos)));
        obj.pos = addV(obj.pos, obj.vel);

        if (testCollision_Obj(sun, obj)) {
            clicked = false;
        }
    }

    if (gameState == 2) {
       moon.pos = addV(moon.pos, moon.vel);
        if (moon.pos.y > 30) {
            moon.vel.x = 1;
            moon.vel.y = 0;
            moon.angle = angleV(moon.pos);
           
        }
        if (moon.pos.x > 150) {
            moon.vel.y = -1;
            moon.angle = angleV(moon.pos);
           
        }
        if (moon.pos.x > 500) {
            moon.pos.x = -600;
            moon.pos.y = -400;
            moon.vel.x = 1;
            moon.vel.y = 1;
            moon.angle = angleV(moon.pos);
            clicked = false;
            gameState = MAIN1;  // after moon is  disappearing again sun will rise for the other day 
           
        }

        vec_t V = { 0, 1 };
        vec_t W = unitV(subV(obj.pos, moon.pos));


        obj.vel = mulV(2, unitV(subV(moon.pos, obj.pos)));
        obj.pos = addV(obj.pos, obj.vel);

        if (testCollision_Obj(moon, obj)) {
            clicked = false;
        }
    }


    glutPostRedisplay(); 

}
#endif

void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("Homework_4_Lighting_Effects_and_Chasing_Simulation_Nurjemal-Saryyeva");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    // keyboard registration
    
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);


    // mouse registration

    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}
/*
 * Athabasca University
 * COMP390 - Computer Graphics
 * Nicholas O'Leary
 * 3466559
 * Final Project
 * Description: 
 *     Deliver a scene that includes
 *      a textured background;
 *      at least five 3D composite objects, each made up of at least five geometric shapes;
 *      at least two light sources with different lighting colours;
 *      an atmospheric attenuation effect.
 *
 * Draws a "pool scene". 
 * The viewing position can be changed using the up/down arrow keys in coordination with
 * dragging the mouse while clicking the left or right mouse buttons.
 *
 * F1, F2, and F3 toggle the three colored spotlights on and off. The lights are located at the 
 * viewing position, pointing forward. When all lights are on they combine to provide white light.
 * toggling them can provide other colors. F1 produces red light, F2 green  light, and F3 blue light.
 *
 * F4 and F5 toggle textures on and off. F4 turns on a texture for the water in the pool.
 * F5 toggles the tiled texture on the walls of the pool.
 *
 * Based on: Unit 8 Section 2 Objective 1 ,Unit 9 Sections 1 Objective 2 by Steve Leung in the 
 *           COMP 390 study guide.
 *
 * White square tile texture attribution: 'https://www.freepik.com/photos/background'
 * Background photo created by rawpixel.com - www.freepik.com
 * Blue pool tile texture from vecteezy.com
 */

#include <Windows.h>
#include "gl/gl.h"
#include "gl/glut.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cassert>
#include "vector3.h"

using namespace std;

#define PI 3.1415926536
bool textured_water = false;
bool plain_walls = false;
bool light_zero = true;
bool light_one = true;
bool light_two = true;


GLfloat light_position0[] = {0.0, 0.0, 0.0, 1.0};
GLfloat light_position1[] = {0.0, 0.0, .0, 1.0};
GLfloat light_position2[] = {0.0, 0.0, 0.0, 1.0};

// Direction vectors;
GLfloat direction_down[] = {0.0, -1.0, 0.0};
GLfloat direction_forward[] = {0.0, 0.0, -1.0};

// Light color vectors.
GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
GLfloat black_light[] = {0.0, 0.0, 0.0, 1.0};
GLfloat red_light[] = {1.0, 0.4, 0.4, 1.0};
GLfloat green_light[] = {0.2, 1.0, 0.2, 1.0};
GLfloat blue_light[] = {0.2, 0.2, 1.0, 1.0};

GLfloat lmodel_ambient[] = {0.1, 0.1, 0.1, 1.0};

// Surface parameters
GLfloat shiny_ambient[] = {0.8, 0.8, 0.8, 1.0};
GLfloat shiny_diffuse[] = {0.6, 0.6, 0.6, 1.0};
GLfloat shiny_specular[] = {0.508273, 0.508273, 0.508273, 1.0};
GLfloat shiny_shininess[] = {100};

GLfloat ambient[] = {0.5, 0.5, 0.5, 1.0};
GLfloat diffuse[] = {0.5, 0.5, 0.5, 1.0};
GLfloat specular[] = {0.2, 0.2, 0.2, 1.0};
GLfloat shininess[] = {1.0};

// A struct to hold the relevant data for one texture image.
struct TEXTURE {
  string fn;
  GLubyte *image;
  GLubyte *l_texture;
  BITMAPFILEHEADER fileheader;
  BITMAPINFOHEADER infoheader;
} texture;

// The initial viewing position and direction.
vector3 viewer = vector3(50, 50, 150);
vector3 lookAt = vector3(0, 0, 0);

// Used to track the mouse position in order to move the camera.
int mouseX, mouseY;

// GLuints for list IDs.
GLuint cube;
GLuint circle;
GLuint cylinder;
GLuint sphere;
GLuint dome;
GLuint triPyramid;
GLuint squarePyramid;
GLuint triPrism;
GLuint poolChair;
GLuint divingBoard;
GLuint hangingLight;
GLuint ladder;
GLuint poolNoodles;

/*
 * Helper function to enable shiny material properties.
 */
void shinyMaterial() {
  glMaterialfv(GL_FRONT, GL_AMBIENT, shiny_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, shiny_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, shiny_diffuse);
  glMaterialfv(GL_FRONT, GL_SHININESS, shiny_shininess);
}

/*
 * Helper function to restore default material properties.
 */
void defaultMaterial() {
  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

/*
 * Helper function to check if gl has encountered an error.
 */
void checkError() {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    cerr << "Error occurred: " << err << " ";
    switch(err) {
    case GL_INVALID_ENUM:
      cerr << "invalid enum";
      break;
    case GL_INVALID_VALUE:
      cerr << "invalid value";
      break;
    case GL_INVALID_OPERATION:
      cerr << "invalid operation";
      break;
    case GL_OUT_OF_MEMORY:
      cerr << "out of memory";
      break;
    case GL_STACK_UNDERFLOW:
      cerr << "stack underflow";
      break;
    case GL_STACK_OVERFLOW:
      cerr << "stack overflow";
      break;
    default:
      cerr << "unknown error";
      break;      
    }
    cerr << endl;
  } 
}

/* 
 * Read an image file into memory and store the texture data
 * in t.l_texture as a sequence of RGBA bytes.
 */
void makeImage(TEXTURE *t) {
  RGBTRIPLE rgb;
  long int i, j;
  FILE *l_file;

  const char * filename = t->fn.c_str();

  // Open image file, return if error.
  fopen_s(&l_file, filename, "rb");
  if (l_file == NULL) return;

  // Read file header and header info.
  fread(&t->fileheader, sizeof(t->fileheader), 1, l_file);
  fseek(l_file, sizeof(t->fileheader), SEEK_SET);
  fread(&t->infoheader, sizeof(t->infoheader), 1, l_file);

  // Allocate space for the image file.
  t->l_texture = (GLubyte *) malloc(t->infoheader.biWidth * t->infoheader.biHeight * 4);
  if (t->l_texture == NULL) return;
  memset(t->l_texture, 0, t->infoheader.biWidth * t->infoheader.biHeight * 4);

  // Read the data.
  j = 0;
  //  int row = 0;
  for (i = 0; i < t->infoheader.biWidth * t->infoheader.biHeight; i++) {
    fread(&rgb, sizeof(rgb), 1, l_file);

    t->l_texture[j] = (GLubyte) rgb.rgbtRed;
    t->l_texture[j+1] = (GLubyte) rgb.rgbtGreen;
    t->l_texture[j+2] = (GLubyte) rgb.rgbtBlue;
    t->l_texture[j+3] = (GLubyte) 0;
    
    j += 4;
  }
  fclose(l_file);
}

/*
 * Helper function to allocate one list id,
 * or exit if glGenLists fails.
 */
GLuint listIdOrExit() {
  GLuint id;
  id = glGenLists(1);
  if (id == 0) {
    cerr << "glGenLists failed. exiting now." << endl;
    exit(1);
  }
  return id;
}
  
/*
 * Define a display list that draws a cube. 
 *
 * The cube has a side length of one and is centered
 * on the origin.
 *
 * Returns the listId.
 */
GLuint makeCube() {
  GLuint id = listIdOrExit();

  vector3 a = vector3(-0.5, -0.5, 0.5);
  vector3 b = vector3(0.5, -0.5, 0.5);
  vector3 c = vector3(0.5, 0.5, 0.5);
  vector3 d = vector3(-0.5, 0.5, 0.5);
  vector3 e = vector3(0.5, -0.5, -0.5);
  vector3 f = vector3(0.5, 0.5, -0.5);
  vector3 g = vector3(-0.5, 0.5, -0.5);
  vector3 h = vector3(-0.5, -0.5, -0.5);
  
// A macro to expand a vector into a glVertex3f  
#define EXP(q) q.x, q.y, q.z  
  
  // Draw a cube with side length of 1, centered on the origin.
  glNewList(id, GL_COMPILE); 

  glBegin(GL_QUADS);
  
  glVertex3f(EXP(b));
  glVertex3f(EXP(e));
  glVertex3f(EXP(f));
  glVertex3f(EXP(c));

  glVertex3f(EXP(h));
  glVertex3f(EXP(a));
  glVertex3f(EXP(d));
  glVertex3f(EXP(g));

  glVertex3f(EXP(a));
  glVertex3f(EXP(b));
  glVertex3f(EXP(c));
  glVertex3f(EXP(d));

  glVertex3f(EXP(e));
  glVertex3f(EXP(h));
  glVertex3f(EXP(g));
  glVertex3f(EXP(f));

  glVertex3f(EXP(c));
  glVertex3f(EXP(f));
  glVertex3f(EXP(g));
  glVertex3f(EXP(d));  

  glVertex3f(EXP(e));
  glVertex3f(EXP(b));
  glVertex3f(EXP(a));
  glVertex3f(EXP(h));  
  
  glEnd();
  
  glEndList();
  
  return id;
}

/*
 * Define a display list that draws a circle.
 * Takes the number of points to use in constructing the 
 * circle as an argument; must be positive.
 * Draws the circle in the xy plane, centered at the origin.
 * 
 * Returns the list id.
 */
GLuint makeCircle(int numPoints) {
  assert(numPoints > 0);

  GLuint id = listIdOrExit();
  
  glNewList(id, GL_COMPILE);
    
  glBegin(GL_POLYGON);  
  for (int i = 0; i < numPoints; i++) {
    double angle = i * (2*PI / numPoints);
    glVertex3d(cos(angle), sin(angle), 0);
  }
  glEnd();

  glEndList();

  return id;
}

/*
 * Defines a display list that draws a cylinder.
 * numPoints is the number of points used to approximate 
 * the circles, it must be positive.
 * Length must be positive.
 * Returns the list Id.
 */
GLuint makeCylinder(int numPoints, int length) {
  assert(length > 0);
  GLuint id = listIdOrExit();

  double *pointsX = new double[numPoints];
  double *pointsY = new double[numPoints];


  for(int i = 0; i < numPoints; i++) {
    double angle = i * (2 * PI / numPoints);
    pointsX[i] = cos(angle);
    pointsY[i] = sin(angle);
  }

  glNewList(id, GL_COMPILE);

  // One end of the cylinder.
  glBegin(GL_POLYGON);
  for(int i = 0; i < numPoints; i++)
    glVertex3d(pointsX[i], pointsY[i], 0);
  glEnd();
  // The other end of the cylinder.
  glBegin(GL_POLYGON);  
  for(int i = numPoints - 1; i >= 0; i--)
    glVertex3d(pointsX[i], pointsY[i], (double)-length);
  glEnd();
  // The middle of the cylinder.
  glBegin(GL_TRIANGLE_STRIP);
  for(int i = 0; i < numPoints; i++){
    if ((i % 2) == 0)
      glVertex3d(pointsX[i], pointsY[i], 0);
    else
      glVertex3d(pointsX[i], pointsY[i], (double)-length);
  }
  glVertex3d(pointsX[0], pointsY[0], 0);
  glVertex3d(pointsX[1], pointsY[1], (double)-length);
  glEnd();
  
  glEndList();

  delete[] pointsX;
  delete[] pointsY;
  
  return id;
}

/*
 * Defines a display list that draws a sphere.
 * Draws the sphere as a "stack of pancakes".
 * numStacks is the number of pancakes, and 
 * numPoints is the number of points used to 
 * approximate the outer edge of each pancake.
 * The sphere is drawn with a radius of 1, centered
 * on the origin. 
 *
 * Returns the list id.
 */
GLuint makeSphere(int numPoints, int numStacks) {
  assert((numPoints > 0) && (numStacks > 0));

  if ((numStacks % 2) != 0) {
    numStacks++;
  }
  
  GLuint id = listIdOrExit();

  double *pointsX = new double[numPoints];
  double *pointsZ = new double[numPoints];  

  for(int i = 0; i < numPoints; i++) {
    double angle = i * (2 * PI / numPoints);
    pointsX[i] = cos(angle);
    pointsZ[i] = sin(angle);
  }  

  glNewList(id, GL_COMPILE);

  for (int stackPair = 0; stackPair < (numStacks / 2); stackPair++) {
    double chordSize = PI / numStacks;
    double outerAngle = stackPair * chordSize;
    double innerAngle = (stackPair + 1) * chordSize;
    double outerCoef = cos(outerAngle);
    double innerCoef = cos(innerAngle);
  
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i < numPoints; i++) {
      glVertex3d(outerCoef * pointsX[i], sin(outerAngle), outerCoef * pointsZ[i]);
      glVertex3d(innerCoef * pointsX[i], sin(innerAngle), innerCoef * pointsZ[i]);
    }
    glVertex3d(outerCoef * pointsX[0], sin(outerAngle), outerCoef * pointsZ[0]);
    glVertex3d(innerCoef * pointsX[1], sin(innerAngle), innerCoef * pointsZ[1]);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i < numPoints; i++) {
      glVertex3d(outerCoef * pointsX[i], -sin(outerAngle), outerCoef * pointsZ[i]);
      glVertex3d(innerCoef * pointsX[i], -sin(innerAngle), innerCoef * pointsZ[i]);
    }
    glVertex3d(outerCoef * pointsX[0], -sin(outerAngle), outerCoef * pointsZ[0]);
    glVertex3d(innerCoef * pointsX[1], -sin(innerAngle), innerCoef * pointsZ[1]);
    glEnd();

  }
  
  // From some angles a gap appears in the sphere.
  // So we add this circle in the centre of the sphere.
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glCallList(circle);
  glPopMatrix();

  glEndList();

  delete[] pointsX;
  delete[] pointsZ;
  
  return id;
}

/*
 * Defines a display list that draws a Dome.
 * Functions the same as makeSphere, but only draws
 * half of  the sphere.
 * Returns the list id.
 */
GLuint makeDome(int numPoints, int numStacks) {
   assert((numPoints > 0) && (numStacks > 0));

  if ((numStacks % 2) != 0) {
    numStacks++;
  }
  
  GLuint id = listIdOrExit();

  double *pointsX = new double[numPoints];
  double *pointsZ = new double[numPoints];  

  for(int i = 0; i < numPoints; i++) {
    double angle = i * (2 * PI / numPoints);
    pointsX[i] = cos(angle);
    pointsZ[i] = sin(angle);
  }  

  glNewList(id, GL_COMPILE);

  for (int stack = 0; stack < (numStacks / 2); stack++) {
    double chordSize = PI / numStacks;
    double outerAngle = stack * chordSize;
    double innerAngle = (stack + 1) * chordSize;
    double outerCoef = cos(outerAngle);
    double innerCoef = cos(innerAngle);
  
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i < numPoints; i++) {
      glVertex3d(outerCoef * pointsX[i], sin(outerAngle), outerCoef * pointsZ[i]);
      glVertex3d(innerCoef * pointsX[i], sin(innerAngle), innerCoef * pointsZ[i]);
    }
    glVertex3d(outerCoef * pointsX[0], sin(outerAngle), outerCoef * pointsZ[0]);
    glVertex3d(innerCoef * pointsX[1], sin(innerAngle), innerCoef * pointsZ[1]);
    glEnd();
  }


  // Draw the base of the dome.
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glCallList(circle);
  glPopMatrix();

  glEndList();

  delete[] pointsX;
  delete[] pointsZ;
  
  return id;
}

/*
 * Define a display list that draws a triangular
 * based pyramid.
 * Returns the list id.
 */
GLuint makeTriPyramid() {
  GLuint id = listIdOrExit();

  vector3 a = vector3(-0.288675, 0, -0.5);
  vector3 b = vector3(-0.288675, 0, 0.5);
  vector3 c = vector3(0.433013, 0 ,0);
  vector3 d = vector3(0, 0.866025, 0);
  
  glNewList(id, GL_COMPILE);
  glBegin(GL_TRIANGLES);
  // Bottom
  glVertex3f(EXP(a));
  glVertex3f(EXP(c));
  glVertex3f(EXP(b));

  glVertex3f(EXP(b));
  glVertex3f(EXP(c));
  glVertex3f(EXP(d));

  glVertex3f(EXP(c));
  glVertex3f(EXP(a));
  glVertex3f(EXP(d));

  glVertex3f(EXP(a));
  glVertex3f(EXP(b));
  glVertex3f(EXP(d));
  
  glEnd();
  glEndList();
  
  return id;
}

/*
 * Define a display list that draws a square
 * based pyramid.
 * Returns the list id.
 */
GLuint makeSquarePyramid() {
  GLuint id = listIdOrExit();

  vector3 a = vector3(0.5, 0, 0.5);
  vector3 b = vector3(-0.5, 0, 0.5);
  vector3 c = vector3(-0.5, 0, -0.5);
  vector3 d = vector3(0.5, 0, -0.5);
  vector3 e = vector3(0, 0.707107, 0);

  glNewList(id, GL_COMPILE);
  glBegin(GL_TRIANGLES);
  // Bottom
  glVertex3f(EXP(a));
  glVertex3f(EXP(b));
  glVertex3f(EXP(c));
  glVertex3f(EXP(c));
  glVertex3f(EXP(d));
  glVertex3f(EXP(a));
  //Sides
  glVertex3f(EXP(a));
  glVertex3f(EXP(d));
  glVertex3f(EXP(e));

  glVertex3f(EXP(d));
  glVertex3f(EXP(c));
  glVertex3f(EXP(e));

  glVertex3f(EXP(c));
  glVertex3f(EXP(b));
  glVertex3f(EXP(e));

  glVertex3f(EXP(b));
  glVertex3f(EXP(a));
  glVertex3f(EXP(e)); 
  
  glEnd();
  glEndList();

  return id;
}

/*
 * Define a display list that draws a triangular Prism.
 * Returns the list id.
 */
GLuint makeTriPrism() {
  GLuint id = listIdOrExit();

  vector3 a = vector3(-0.5, 0, -0.5);
  vector3 b = vector3(-0.5, 0, 0.5);
  vector3 c = vector3(0.5, 0, 0.5);
  vector3 d = vector3(0.5, 0, -0.5);
  vector3 e = vector3(0.5, 0.866025, 0);
  vector3 f = vector3(-0.5, 0.866025, 0);

  glNewList(id, GL_COMPILE);
  
  glBegin(GL_TRIANGLES);
  // End Faces
  glVertex3f(EXP(c));
  glVertex3f(EXP(d));
  glVertex3f(EXP(e));

  glVertex3f(EXP(a));
  glVertex3f(EXP(b));
  glVertex3f(EXP(f));  
  glEnd();

  // Middle Faces
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(EXP(b));
  glVertex3f(EXP(c));
  glVertex3f(EXP(f));
  glVertex3f(EXP(e));
  glVertex3f(EXP(a));
  glVertex3f(EXP(d));
  glVertex3f(EXP(b));
  glVertex3f(EXP(c));  
  glEnd();
  
  glEndList();  

  return id;
}

/*
 * Define a display list to draw a pool chair.
 * Returns the list id.
 */
GLuint makePoolChair() {
  GLuint frame = listIdOrExit();
  
  glNewList(frame, GL_COMPILE);
  
  glPushMatrix();
  glScalef(0.5, 0.5, 2.0);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -3, -17);
  glRotatef(45.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 0.4);
  glCallList(cylinder);
  glPopMatrix();


  glPushMatrix();
  glTranslatef(0.0, -0.2, 0.0);
  glRotatef(-45.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 0.4);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -3, -5.0);
  glRotatef(45.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 0.4);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -0.2, -10.0);
  glRotatef(-45.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 0.4);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -2.9, -2.5);
  glScalef(0.5, 0.5, 0.27);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -2.9, -12.5);
  glScalef(0.5, 0.5, 0.47);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -0.05, 0.05);
  glScalef(0.525, 0.525, 0.525);
  glCallList(sphere);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -0.05, -20.05);
  glScalef(0.525, 0.525, 0.525);
  glCallList(sphere);
  glPopMatrix();     
    
  glEndList();

  
  GLuint id = listIdOrExit();

  glNewList(id, GL_COMPILE);

  glColor4f(1.0, 1.0, 1.0, 0.0);
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glScalef(10.0, 20.0, 1.0);
  glCallList(cube);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 5.0, -14.0);
  glRotatef(-45.0, 1.0, 0.0, 0.0);
  glScalef(10.0, 10.0, 1.0);  
  glCallList(cube);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.5, 0.0, 9.5);
  glCallList(frame);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-5.5, 0.0, 9.5);
  glCallList(frame);
  glPopMatrix();


  
  glEndList(); 
  
  return id;
}

/*
 * Define a display list to draw a diving board.
 * Returns the list id.
 */
GLuint makeDivingBoard() {
  GLuint railing = listIdOrExit();
  
  glNewList(railing, GL_COMPILE);
  glColor4f(0.5, 0.5, 0.5, 0.0);
  
  shinyMaterial();
  
  glPushMatrix();
  glScalef(0.5, 0.5, 3.0);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -12.5, -15.0);
  glRotatef(45.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 1.75);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.0, -1.0);
  glRotatef(-60.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 1.5);
  glCallList(cylinder);
  glPopMatrix();

  defaultMaterial();
  glEndList();
  
  GLuint id = listIdOrExit();

  glNewList(id, GL_COMPILE);  
  glColor4f(0.8, 0.8, 1.0, 0.0);
  glPushMatrix();
  glScalef(10.0, 15.0, 15.0);
  glCallList(cube);
  glPopMatrix();

  glColor4f(0.8, 1.0, 0.8, 0.);
  glPushMatrix();
  glTranslatef(0.0, 8.0, 0.0);
  glScalef(15.0, 1.0, 25.0);
  glCallList(cube);
  glPopMatrix();

  
  glPushMatrix();
  glTranslatef(0.0, 3.0, 10.0);
  glRotatef(-30.0, 1.0, 0.0, 0.0);
  glScalef(10.0, 12.0, 1.0);
  glCallList(cube);
  glPopMatrix();

  glColor4f(0.7, 1.0, 0.7, 0.0);
  glPushMatrix();
  glTranslatef(0.0, -1.75, 13.8);
  glRotatef(-60.0, 1.0, 0.0, 0.0);
  glScalef(9.5, 2.0, 2.0);
  glCallList(triPrism);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.5, 12.8);
  glRotatef(-60.0, 1.0, 0.0, 0.0);
  glScalef(9.5, 2.0, 2.0);
  glCallList(triPrism);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 2.75, 11.5);
  glRotatef(-60.0, 1.0, 0.0, 0.0);
  glScalef(9.5, 2.0, 2.0);
  glCallList(triPrism);
  glPopMatrix();

  glColor4f(1.0, 1.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(0.0, 9.0, -22.4);
  glScalef(10.0, 1.0, 70.0);
  glCallList(cube);
  glPopMatrix();  

  glPushMatrix();
  glTranslatef(-5.5, 20.0, 14.0);
  glCallList(railing);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.5, 20.0, 14.0);
  glCallList(railing);
  glPopMatrix();
  
  glEndList();
  return id;
}

/*
 * Define a display list to draw a hanging light.
 * returns the list id.
 */
GLuint makeHangingLight() {
  GLuint id = listIdOrExit();

  glNewList(id, GL_COMPILE);
  glColor4f(1.0, 1.0, 1.0, 0.0);
  glPushMatrix();
  // Allow the "globe" of the light to emit light.
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, white_light);
  glCallList(sphere);  
  glPopMatrix();

  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black_light);

  
  glColor4f(0.1, 0.1, 0.1, 0.0);
  glPushMatrix();
  glTranslatef(0.0, 1.0, 0.0);
  glRotatef(90, 1.0, 0.0, 0.0);
  glScalef(0.2, 0.2, 1.0);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.15, 0.0);
  glScalef(1.5, 1.5, 1.5);
  glCallList(dome);
  glPopMatrix();
  
  glEndList();
  
  return id;
}

/*
 * Define a display list to draw a ladder.
 * Returns the list id.
 */
GLuint makeLadder() {

  GLuint rung = listIdOrExit();
  
  glNewList(rung, GL_COMPILE);
  
  glColor4f(0.5, 0.5, 0.5, 0.0);
  glPushMatrix();
  glScalef(8.0, 1.0, 3.0);
  glCallList(cube);
  glPopMatrix();

  glPushMatrix();
  glColor4f(0.4, 0.4, 0.4, 0.0);
  glCallList(dome);
  glTranslatef(-3.0, 0.0, 0.0);
  glCallList(dome);
  glTranslatef(6.0, 0.0, 0.0);
  glCallList(dome);
  glPopMatrix();
    
  glEndList();
  
  GLuint railing = listIdOrExit();
  
  glNewList(railing, GL_COMPILE);
  shinyMaterial();  
  glColor4f(0.5, 0.5, 0.5, 0.0);
  
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 4.0);  
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 40.0, 0.0);
  glScalef(0.5, 0.5, 1.0);
  glCallList(cylinder);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 40.0, 0.0);
  glScalef(0.5, 0.5, 0.5);
  glCallList(sphere);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 25.0, -10.0);
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.5, 1.5);
  glCallList(cylinder);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 40.0, -10.0);
  glScalef(0.5, 0.5, 0.5);
  glCallList(sphere);
  glPopMatrix();
  defaultMaterial();
  glEndList();
  
  GLuint id = listIdOrExit();
  glNewList(id, GL_COMPILE);
  shinyMaterial();

  glPushMatrix();
  glTranslatef(0.0, -5.0, 0.0);  
  glCallList(rung);
  glTranslatef(0.0, 5.0, 0.0);
  glCallList(rung);
  glTranslatef(0.0, 5.0, 0.0);
  glCallList(rung);
  glTranslatef(0.0, 5.0, 0.0);
  glCallList(rung);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(4.0, -10.0, 0.0);
  glCallList(railing);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-4.0, -10.0, 0.0);
  glCallList(railing);
  glPopMatrix();


  defaultMaterial();
  glEndList();
  
  return id;
}

/*
 * Helper function to draw one pool noodle.
 */
void drawPoolNoodle(vector3 color) {
  glColor4f(((color.x - 0.2) < 0.0) ? 0.0 : color.x - 0.2,
	    ((color.y - 0.2) < 0.0) ? 0.0 : color.y - 0.2,
	    ((color.z - 0.2) < 1.0) ? 0.0 : color.z - 0.2,
	    0);
  
  glPushMatrix();
  glTranslatef(0.0, 0.0, 0.2);
  glScalef(0.2, 0.2, 0.2);
  glCallList(circle);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.0, -25.2);
  glRotatef(180.0, 0.0, 1.0, 0.0);
  glScalef(0.2, 0.2, 0.2);
  glCallList(circle);
  glPopMatrix();

  
  glColor4f(color.x, color.y, color.z, 0.0);
  glPushMatrix();
  glScalef(1.0, 1.0, 2.5);
  glCallList(cylinder);
  glPopMatrix();
}

/*
 * Define a display list to  draw some pool noodles.
 * Returns the list id.
 */
GLuint makePoolNoodles() {
  
  vector3 red = {1.0, 0.0, 0.0};
  vector3 green = {0.0, 1.0, 0.0};
  vector3 blue = {0.0, 0.0, 1.0};
  vector3 pink = {1.0, 0.5, 0.5};
  vector3 purple = {1.0, 0.0, 1.0};
  vector3 yellow = {1.0, 1.0, 0.0};
    
  
  GLuint id = listIdOrExit();
  
  glNewList(id, GL_COMPILE);
  
  glPushMatrix();
  drawPoolNoodle(red);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(2.0, 0.0, 0.0);
  drawPoolNoodle(green);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(4.0, 0.0, 0.0);
  drawPoolNoodle(blue);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(6.0, 0.0, 0.0);
  drawPoolNoodle(pink);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(8.0, 0.0, 0.0);
  drawPoolNoodle(purple);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(10.0, 0.0, 0.0);
  drawPoolNoodle(yellow);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(1.0, 2.0, 0.0);
  drawPoolNoodle(pink);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(3.0, 2.0, 0.0);
  drawPoolNoodle(yellow);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.0, 2.0, 0.0);
  drawPoolNoodle(red);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(7.0, 2.0, 0.0);
  drawPoolNoodle(blue);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(9.0, 2.0, 0.0);
  drawPoolNoodle(green);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(2.0, 4.0, 0.0);
  drawPoolNoodle(purple);
  glPopMatrix();


  glPushMatrix();
  glTranslatef(4.0, 4.0, 0.0);
  drawPoolNoodle(pink);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(6.0, 4.0, 0.0);
  drawPoolNoodle(green);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(8.0, 4.0, 0.0);
  drawPoolNoodle(red);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(3.0, 6.0, 0.0);
  drawPoolNoodle(blue);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.0, 6.0, 0.0);
  drawPoolNoodle(yellow);  
  glPopMatrix();

  glPushMatrix();
  glTranslatef(7.0, 6.0, 0.0);
  drawPoolNoodle(purple);
  glPopMatrix();
  
  glEndList();  
  return id;
}


/*
 * Initialize. Set up the required parameters for the program.
 */
void initialize() {
  glClearColor(0.2, 0.5, 0.2, 0.0);
  /*
   * Display Lists
   */
  cube = makeCube();
  circle = makeCircle(100);
  cylinder = makeCylinder(100, 10);
  sphere = makeSphere(100, 50);
  dome = makeDome(100, 50);
  triPyramid = makeTriPyramid();
  squarePyramid = makeSquarePyramid();
  triPrism = makeTriPrism();
  poolChair = makePoolChair();
  divingBoard = makeDivingBoard();
  hangingLight = makeHangingLight();
  ladder = makeLadder();
  poolNoodles = makePoolNoodles();

  /*
   * Texture Image
   */
  makeImage(&texture);  // Load the texture into memory.

  GLuint texName;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Specify that each pixel row in memory will be byte aligned.

  glGenTextures(1, &texName); // Generate a texture name.
  glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture name to a 2D texture object.
 
  // Repeat the texture in the s direction if necessary.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // Repeat the texture in the t direction if necessary.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Use linear interpolation when magnifying the texture.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Use linear interpolation when minifying the texture.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // Tell the texture function to simply replace the pixel color
  // with the color as its stored in the texture. Alternatively
  // we could specify how the source and destination colors are blended together.  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // Define the texture image.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	       texture.infoheader.biWidth, texture.infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
	       texture.l_texture);

  /*
   * OpenGL Paramters
   */
  glEnable(GL_CULL_FACE); // Turn on back face culling.
  glEnable(GL_DEPTH_TEST);// Turn on depth buffer visible surface detection
  // Enable Transparency 
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
  
  glEnable(GL_FOG); // Enable atmospheric attenuation.
  GLfloat atmoColor [4] = {0.6, 0.6, 0.9, 1.0}; // Grey/Blue
  glFogfv(GL_FOG_COLOR, atmoColor); // Set the fog color.
  glFogi(GL_FOG_MODE, GL_EXP2);     // Calculate the blending factor for the fog using the equation
                                    // f = e^(-density*c^2)
  glFogf(GL_FOG_DENSITY, 0.005);   // Set the density value for calculating f.


  /*
   * Lights
   */
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);

  // Tell the lighting model to take into account a material's color as well as its
  // lighting surface properties.
  glEnable(GL_COLOR_MATERIAL);

  glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

  glLightfv(GL_LIGHT0, GL_AMBIENT, red_light);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, red_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, red_light);

  // Make a spotlight with a cone of 90 degrees, and attenuation exponent of one.
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction_forward);
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0);
  glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);

  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

  glLightfv(GL_LIGHT1, GL_AMBIENT, green_light);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, green_light);
  glLightfv(GL_LIGHT1, GL_SPECULAR, green_light);


  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction_forward);
  glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0);
  glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1.0);

  glLightfv(GL_LIGHT2, GL_POSITION, light_position2);

  glLightfv(GL_LIGHT2, GL_AMBIENT, blue_light);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, blue_light);
  glLightfv(GL_LIGHT2, GL_SPECULAR, blue_light);

  glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction_forward);
  glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 90.0);
  glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 1.0);

  // Set the ambient lighting model parameters to the values
  // stored in lmodel ambient.
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

  // Set the material properties.
  defaultMaterial();
}

/* 
 * The texture we loaded combines four textures.
 * This enum is used to select which texture we want to 
 * work with.
 */ 
enum Texture {White, Blue, Water, Green, None};

/*
 * Draw a textured or non-textured rectangle.
 * (x1, y1, z1) is the lower left corner and
 * (x2, y2, z2) is the upper right corner.
 * yz specifies if the rectangle is to be drawn in the yz plane.
 * t selects which texture (if any) to display on the rectangle.
 */
void tileRect(float x1, float y1, float z1, float x2, float y2, float z2, bool yz, Texture t) {
  // The texCoords of each texture.
  static float whiteCoords[4][2] = { {0, 0.5}, {0.5, 0.5}, {0.5, 1}, {0, 1} };
  static float blueCoords[4][2] = { {0.5, 0.5}, {1, 0.5}, {1, 1}, {0.5, 1} };
  static float greenCoords[4][2] = { {0.5, 0}, {1, 0}, {1, 0.5}, {0.5, 0.5} };

  bool use_no_texture = false;
  
  float (*coords)[2];

  switch (t){
  case White:
    coords  = whiteCoords;
    break;
  case Blue:
    coords = blueCoords;
    break;
  case Green:
    coords = greenCoords;
    break;
  case None:
    coords = whiteCoords; // Avoids a compiler warning.
    use_no_texture = true;
    break;
  default:
    coords = NULL;
    break;
  }

  if (coords == NULL) {
    cerr << "Invalid argument to tileRect. t must be one of White, Blue, Green, or None." << endl;
    exit(1);
  }
  if (use_no_texture) {
    if (yz) {
      glBegin(GL_QUADS);
      glVertex3f(x1, y1, z1);
      glVertex3f(x2, y1, z2);
      glVertex3f(x2, y2, z2);
      glVertex3f(x1, y2, z1);
      glEnd();
    } else {  
      glBegin(GL_QUADS);
      glVertex3f(x1, y1, z1);
      glVertex3f(x2, y1, z1);
      glVertex3f(x2, y2, z2);
      glVertex3f(x1, y2, z2);
      glEnd();
    }
  } else {
    glEnable(GL_TEXTURE_2D);
  
    if (yz) {
      glBegin(GL_QUADS);
      glTexCoord2d(coords[0][0], coords[0][1]);
      glVertex3f(x1, y1, z1);
      glTexCoord2d(coords[1][0], coords[1][1]);
      glVertex3f(x2, y1, z2);
      glTexCoord2d(coords[2][0], coords[2][1]);
      glVertex3f(x2, y2, z2);
      glTexCoord2d(coords[3][0], coords[3][1]);
      glVertex3f(x1, y2, z1);
      glEnd();

    } else {  
      glBegin(GL_QUADS);
      glTexCoord2d(coords[0][0], coords[0][1]);
      glVertex3f(x1, y1, z1);
      glTexCoord2d(coords[1][0], coords[1][1]);
      glVertex3f(x2, y1, z1);
      glTexCoord2d(coords[2][0], coords[2][1]);
      glVertex3f(x2, y2, z2);
      glTexCoord2d(coords[3][0], coords[3][1]);
      glVertex3f(x1, y2, z2);
      glEnd();
    }

    glDisable(GL_TEXTURE_2D);
  }
}

/*
 * Draws the water in the pool using a bezier spline surface.
 *
 * Based on chapter 14 from the text.
 *
 */
void renderSplineSurface() {
  // We specify 16 control points inside the pool.
  static const GLfloat points[4][4][3] = {
    { {-50, -10, 100}, {-25, -5, 100},
      {20, -8, 100}, {50, -15, 100} },
    { {-50, -20, 66}, {-20, -20, 66},
      {20, -30, 66}, {50, -10, 66} },
    { {-50, -9, -66}, {-30, -20, -66},
      {0, -10, -66}, {50, -15, -66} },
    { {-50, -15, -100}, {-22, -20, -100},
      {15, -30, -100}, {50, -20, -100} }
  };

  // Texture coords for the water texture.
  static const GLfloat waterCoords[2][2][2] = { {{0, 0}, {0, 0.5}},
						{{0.5, 0}, {0.5, 0.5}} };

  glMap2f(GL_MAP2_VERTEX_3, 0.0, 1.0, 12, 4,
	  0.0, 1.0, 3, 4, &points[0][0][0]); // Map our control points.
  if (textured_water) {
    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, // And map our texture coordinates.
	    0, 1, 4, 2, &waterCoords[0][0][0]);
    glEnable(GL_MAP2_TEXTURE_COORD_2);
    glEnable(GL_TEXTURE_2D); 
  }
  glEnable(GL_MAP2_VERTEX_3);

  glColor4f(0.0, 0.0, 1.0, 0.3);

  // Defines a mesh derived from our control points with 20 vertices in the u and v directions.  
  glMapGrid2f(20, 0, 1, 20, 0, 1);
  // Draws the vertices defined by our mesh as if we called glBegin(GL_QUAD_STRIP)
  glEvalMesh2(GL_FILL, 0, 20, 0, 20);

  glDisable(GL_TEXTURE_2D);

  checkError();
}

/*
 * Render the scene.
 * The various preprocessor directives were used in developing
 * each component of the program. Enabling them will draw 
 * different components centred at the origin.
 *
 * The tile floor and walls of the pool have a translucent 
 * non-textured quad drawn over top of them. This amplifies the lighting
 * effects, as lighting variations are less apparent on the textures alone.
 *
 */
void render() {
  /*
   * Draw the blue tiled floor.
   */

  // Close short side.
  tileRect(-50, 0, 150, 0, 0, 100, false, Blue);
  tileRect(0, 0, 150, 50, 0, 100, false, Blue);

  // Right long side.
  tileRect(50, 0, 150, 100, 0, 100, false, Blue);
  tileRect(50, 0, 100, 100, 0, 50, false, Blue);
  tileRect(50, 0, 50, 100, 0, 0, false, Blue);
  tileRect(50, 0, 0, 100, 0, -50, false, Blue);
  tileRect(50, 0, -50, 100, 0, -100, false, Blue);
  tileRect(50, 0, -100, 100, 0, -150, false, Blue);

  // Far short side.
  tileRect(0, 0, -100, 50, 0, -150, false, Blue);
  tileRect(-50, 0, -100, 0, 0, -150, false, Blue);

  // Left long side.
  tileRect(-100, 0, -100, -50, 0, -150, false, Blue);
  tileRect(-100, 0, -50, -50, 0, -100, false, Blue);
  tileRect(-100, 0, 0, -50, 0, -50, false, Blue);
  tileRect(-100, 0, 50, -50, 0, 0, false, Blue);
  tileRect(-100, 0, 100, -50, 0, 50, false, Blue);
  tileRect(-100, 0, 150, -50, 0, 100, false, Blue);

  /*
   * Alpha layer
   */
  glColor4f(1.0, 1.0, 1.0, 0.5);
    // Close short side.
  tileRect(-50, 0.1, 150, 0, 0.1, 100, false, None);
  tileRect(0, 0.1, 150, 50, 0.1, 100, false, None);

  // Right long side.
  tileRect(50, 0.1, 150, 100, 0.1, 100, false, None);
  tileRect(50, 0.1, 100, 100, 0.1, 50, false, None);
  tileRect(50, 0.1, 50, 100, 0.1, 0, false, None);
  tileRect(50, 0.1, 0, 100, 0.1, -50, false, None);
  tileRect(50, 0.1, -50, 100, 0.1, -100, false, None);
  tileRect(50, 0.1, -100, 100, 0.1, -150, false, None);

  // Far short side.
  tileRect(0, 0.1, -100, 50, 0.1, -150, false, None);
  tileRect(-50, 0.1, -100, 0, 0.1, -150, false, None);

  // Left long side.
  tileRect(-100, 0.1, -100, -50, 0.1, -150, false, None);
  tileRect(-100, 0.1, -50, -50, 0.1, -100, false, None);
  tileRect(-100, 0.1, 0, -50, 0.1, -50, false, None);
  tileRect(-100, 0.1, 50, -50, 0.1, 0, false, None);
  tileRect(-100, 0.1, 100, -50, 0.1, 50, false, None);
  tileRect(-100, 0.1, 150, -50, 0.1, 100, false, None);


  /*
   * Pool Sides and Bottom.
   */

  // Near Side
  tileRect(50, -100, 100, -50, 0, 100, false, White);
  // Right Side
  tileRect(50, -100, -100, 50, 0, 100, true, White);
  // Far Side
  tileRect(-50, -100, -100, 50, 0, -100, false, White);
  // Left Side
  tileRect(-50, -100, 100, -50, 0, -100, true, White);
  // Bottom
  tileRect(-50, -100, 100, 50, -100, -100, false, White);

  /*
   * Walls of the Pool.
   */
  glColor4f(1.0, 1.0, 1.0, 0.0);
  float delta = 0.5;
  if (plain_walls) {
  // Near Wall
  tileRect(100, 0, 150, -100, 100, 150, false, None);
  // Right Wall
  tileRect(100, 0, -150, 100, 100, 150, true, None);
  // Far Wall
  tileRect(-100, 0, -150, 100, 100, -150, false, None);
  // Left Wall
  tileRect(-100, 0, 150, -100, 100, -150, true, None);
  } else {
    // Near Wall
    tileRect(100, 0, 150, -100, 100, 150, false, White);
    // Right Wall
    tileRect(100, 0, -150, 100, 100, 150, true, White);
    // Far Wall
    tileRect(-100, 0, -150, 100, 100, -150, false, White);
    // Left Wall
    tileRect(-100, 0, 150, -100, 100, -150, true, White);
  }
  /*
   * Alpha Layer
   */

  glColor4f(1.0, 1.0, 1.0, 0.3);
  tileRect(100, 0, 150 - delta, -100, 100, 150 - delta, false, None);
  tileRect(100 - delta, 0, -150, 100 - delta, 100, 150, true, None);
  tileRect(-100, 0, -150 + delta, 100, 100, -150 + delta, false, None);
  tileRect(-100 + delta, 0, 150, -100 + delta, 100, -150, true, None);
  
  /*
   * Ceiling of the pool
   */

  glColor4f(0.1, 0.1, 0.1, 0.0);
  
  glBegin(GL_QUADS);
  glVertex3f(100.0, 100.0, 150.0);
  glVertex3f(-100.0, 100.0, 150.0);
  glVertex3f(-100.0, 100.0, -150.0);
  glVertex3f(100.0, 100.0, -150.0);  
  glEnd();

  
  //#define TEST_SHAPES 
#ifdef TEST_SHAPES
  /*
   * Draw the cube.
   */
  glColor4f(1.0, 0.0, 0.0, 0.0);
  glPushMatrix();
  glTranslatef(20.0, 0.0, 0.0);
  glScalef(10.0, 10.0, 10.0);
  glCallList(cube);
  glPopMatrix();
  

  /*
   * Draw the circle.
   */  
  glColor4f(0.0, 1.0, 0.0, 0.0);
  glPushMatrix();
  glTranslatef(0.0, 0.0, -20.0);
  glScalef(5.0, 5.0, 5.0);
  glCallList(circle);
  glPopMatrix();


  /*
   * Draw the cylinder
  */
  glColor4f(0.0, 0.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(-20.0, 0.0, 0.0);
  glCallList(cylinder);
  glPopMatrix();


  /*
   * Draw the sphere
  */

  glColor4f(1.0, 0.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(20.0, 30.0, 0.0);
  glScalef(5.0, 5.0, 5.0);
  glCallList(sphere);
  glPopMatrix();


  /*
   * Draw the dome.
  */

  glColor4f(1.0, 1.0, 0.0, 0.0);
  glPushMatrix();
  glTranslatef(-20.0, 30.0, 0.0);
  glScalef(5.0, 5.0, 5.0);
  glCallList(dome);
  glPopMatrix();

  /*
   * Draw the Triangular Pyramid.
   */
  glColor4f(0.0, 1.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(0.0, 30.0, -30.0);
  glScalef(10.0, 10.0, 10.0);
  glCallList(triPyramid);
  glPopMatrix();

  /*
   * Draw the Square based Pyramid.
   */
  glColor4f(1.0, 0.2, 0.2, 0.0);
  glPushMatrix();
  glTranslatef(-20.0, 0.0, 20.0);
  glScalef(10.0, 10.0, 10.0);
  glCallList(squarePyramid);
  glPopMatrix();
 
  /*
   * Draw the triangular prism.
   */
  glColor4f(0.2, 1.0, 0.2, 0.0);
  glPushMatrix();
  glTranslatef(20.0, 0.0, 20.0);
  glScalef(5.0, 5.0, 5.0);
  glCallList(triPrism);
  glPopMatrix();
    
  
#endif // TEST_SHAPES

  //#define TEST_POOL_CHAIR
#ifdef TEST_POOL_CHAIR
  glPushMatrix();
  glCallList(poolChair);
  glPopMatrix();  
#endif // TEST_POOL_CHAIR

  //#define TEST_DIVING_BOARD  
#ifdef TEST_DIVING_BOARD
  glPushMatrix();
  glCallList(divingBoard);
  glPopMatrix();
#endif // TEST_DIVING_BOARD

  //#define TEST_HANGING_LIGHT
#ifdef TEST_HANGING_LIGHT
  glPushMatrix();
  glCallList(hangingLight);
  glPopMatrix();
#endif // TEST_HANGING_LIGHT

  //#define TEST_LADDER
#ifdef TEST_LADDER
  glPushMatrix();
  glCallList(ladder);
  glPopMatrix();
#endif // TEST_LADDER

//#define TEST_POOL_NOODLES
#ifdef TEST_POOL_NOODLES
  glPushMatrix();
  glCallList(poolNoodles);
  glPopMatrix();
#endif // TEST_POOL_NOODLES

#define DRAW_THE_SCENE
#ifdef DRAW_THE_SCENE
  // Draw the Ladder.
  glPushMatrix();
  glTranslatef(-48.0, -20.0, -90.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  glCallList(ladder);
  glPopMatrix();

  // Draw some pool chairs
  glPushMatrix();
  glTranslatef(-80.0, 3.0, 0.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();
  
  glPushMatrix();
  glTranslatef(-80.0, 3.0, 70.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-80.0, 3.0, -70.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(80.0, 3.0, 0.0);
  glRotatef(270.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();
  
  glPushMatrix();
  glTranslatef(80.0, 3.0, 70.0);
  glRotatef(270.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(80.0, 3.0, -70.0);
  glRotatef(270.0, 0.0, 1.0, 0.0);
  glCallList(poolChair);
  glPopMatrix();

  // Draw the diving board
  glPushMatrix();
  glTranslatef(0.0, 8.0, 115.0);
  glCallList(divingBoard);
  glPopMatrix();

  // Draw the lights
  
  glPushMatrix();
  glTranslatef(-55.0, 55.0, 50.0);
  glScalef(4.0, 4.0, 4.0);
  glCallList(hangingLight);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 55.0, 0.0);
  glScalef(4.0, 4.0, 4.0);
  glCallList(hangingLight);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(55.0, 55.0, -50.0);
  glScalef(4.0, 4.0, 4.0);
  glCallList(hangingLight);
  glPopMatrix();

  // Draw a stack of pool noodles
  glPushMatrix();
  glTranslatef(70.0, 2.0, -135.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  glCallList(poolNoodles);
  glPopMatrix();
#endif // DRAW_THE_SCENE
  
  /*
   * Render the Water
   */
  renderSplineSurface();
  glFlush();
}

/*
 * Display Registry
 */
void display(void) {
  // Clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Make the viewing matrix the identity matrix.
  glLoadIdentity();
  // Set viewing matrix to look at the "lookAt" vector from "viewer" with
  // the positive y axis as the up direction.
  gluLookAt(viewer.x, viewer.y, viewer.z, lookAt.x, lookAt.y, lookAt.z, 0, 1, 0);
  // Draw the scene
  render();
  // Display the update by swapping the front and back buffers.
  glutSwapBuffers();
}

/*
 * Reshape registry.
 */
void reshape(int w, int h) {
  // Set the viewport to the new size of the window.
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  // Set the matrix mode to modify the projection matrix.
  glMatrixMode(GL_PROJECTION);
  // Load the identity matrix in to the projection matrix.
  glLoadIdentity();
  // Set the projection matrix to a normalized frustum from -1 to 1
  // in the x and y direction, with a near clipping plane of 1.5 units
  // and a far clipping plane of 200 units.
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 1000.0);
  // Set the matrix mode back to modelview.
  glMatrixMode(GL_MODELVIEW);
}

/*
 * Simple vector3 times 3x3 matrix multiplication for use in moveViewer.
 */
vector3 vector3TimesMatrix3x3(vector3 v, float* m) {
  vector3 result = {v.x * m[0] + v.y * m[3] + v.z * m[6],
                    v.x * m[1] + v.y * m[4] + v.z * m[7],
                    v.x * m[2] + v.y * m[5] + v.z * m[8]};
  return result;
}

/*
 * Motion registry. Callback function to move the lookAt vector
 * whenever the mouse is moved with one of the mouse buttons held down.
 */
void moveLookAt(int x, int y) {
  int dX = x - mouseX;
  int dY = y - mouseY;

  float yRotation = dX / 20.0;
  float xRotation = dY / 20.0;

  // Rotate about the y axis to move the camera left and right.
  float yRotationMatrix[] =  {cos(yRotation), 0, sin(yRotation),
    0, 1,              0,
    -1 * sin(yRotation), 0, cos(yRotation)};
  // Rotate about the x axis to mvoe the camera up and down.
  float xRotationMatrix[] = { 1,              0,                   0,
    0, cos(xRotation), -1 * sin(xRotation),
    0, sin(xRotation),      cos(xRotation)};
  
  // Rotate the direction we are looking.
  vector3 dirVec = vector3TimesMatrix3x3(vector3TimesMatrix3x3(lookAt.subtract(viewer).normalize(),
							       yRotationMatrix), xRotationMatrix);
  // Update the location we are looking at.
  lookAt = viewer.add(dirVec);
  
  mouseX = x;
  mouseY = y;
  
  glutPostRedisplay();
}

/*
 * Keyboard motion registry. Callback function to move the viewer position
 * using the arrow keys of the keyboard.
 * Also handles user input for toggling lights and textures.
 */
void moveViewer(int key, int x, int y) {
  vector3 dirVec = {0, 0, 0}; 
  switch (key) {
  case GLUT_KEY_UP:
    dirVec = lookAt.subtract(viewer).normalize();
    break;
  case GLUT_KEY_DOWN:
    dirVec = lookAt.subtract(viewer).normalize().scalar(-1);
    break;
  case GLUT_KEY_F1:
    if (light_zero)
      glDisable(GL_LIGHT0);
    else glEnable(GL_LIGHT0);
    light_zero = !light_zero;
    break;
  case GLUT_KEY_F2:
    if (light_one) 
      glDisable(GL_LIGHT1);
    else glEnable(GL_LIGHT1);    
    light_one = !light_one;
    break;
  case GLUT_KEY_F3:
    if (light_two)
      glDisable(GL_LIGHT2);
    else glEnable(GL_LIGHT2);
    light_two = !light_two;
    break;
  case GLUT_KEY_F4:
    textured_water = !textured_water;
    break;
  case GLUT_KEY_F5:
    plain_walls = !plain_walls;
    break;
  }

  viewer = viewer.add(dirVec);
  lookAt = lookAt.add(dirVec);
  
  glutPostRedisplay();
}

/*
 * Passive Motion Registry. Tracks current mouse location.
 */
void trackMouse(int x, int y) {
  mouseX = x;
  mouseY = y;
}

/*
 * Main program.
 */
int main(int argc, char** argv) {
  // Texture info.
  texture.fn = "combined-texture.bmp"; // 2800 * 1960  
  
  // Initialize glut.
  glutInit(&argc, argv);
  // Set the initial display mode to use double buffering, and
  // to allocate both a color buffer and a depth buffer.  
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  // Set the initial window size.
  glutInitWindowSize(750, 750);
  // Set the initial window position.
  glutInitWindowPosition(100, 100);
  
  // Create the window and give it a title.
  int windowHandle = glutCreateWindow("Final Project");
  glutSetWindow(windowHandle);

  // Assign the callback functions.
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(moveViewer);
  glutPassiveMotionFunc(trackMouse);
  glutMotionFunc(moveLookAt);

  // Set our program's parameters.
  initialize();

  // Draw the scene until the window is close.
  glutMainLoop();
  
  return 0;
}

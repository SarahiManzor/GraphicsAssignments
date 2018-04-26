/*Manzor Sarahi
500645416*/
/*******************************************************************
Multi-Part Model Construction and Manipulation
********************************************************************/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

// Lighting/shading and material properties for submarine - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat submarine_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat submarine_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat submarine_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat submarine_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground / sea floor 
static QuadMesh groundMesh;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void myDrawShip();
void update();

//My variables
float xMovement;
float yMovement;
float zMovement;

float yRotation = 0;
float propellorRotation = 0;

float shipSpeed = 0.001f;

float velocity = 0;

int view = 0;


int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	glutTimerFunc(10, update, 0);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

void update()
{
	if (velocity == 0) {
		propellorRotation += 0;
	}
	else {
		propellorRotation += (velocity > 0 ? 2 : -2) + 50 * velocity;
	}
	float zChange = velocity * sin(yRotation * 3.14159265 / 180);
	float xChange = velocity * cos(yRotation * 3.14159265 / 180);
	//printf("X: %f\n", xChange);
	//printf("Z: %f\n", zChange);
	zMovement += zChange;
	xMovement -= xChange;
	display();
	glutTimerFunc(1000/60, update, 0);
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

														 // Set up ground/sea floor quad mesh
	Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);
}

void mySolidCylinder(GLUquadric *qobj,GLdouble baseRadius,GLdouble topRadius,GLdouble height,GLint slices,GLint stacks) {
	gluCylinder(qobj, baseRadius, topRadius, height, slices, stacks);
	gluDisk(qobj, 0, topRadius, slices, stacks);
	glTranslatef(0, 0, height);
	gluDisk(qobj, 0, baseRadius, slices, stacks);
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Set up the camera at position (top, side or follow)
	switch (view) {
		case 0:
			gluLookAt(0, 20, 1, 0, 0, 0, 0, 1, 0);
			break;
		case 1:
			gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);
			break;
		case 2:
			gluLookAt(xMovement + 6 * cos(yRotation * 3.14159265 / 180), yMovement, zMovement - 6 * sin(yRotation * 3.14159265 / 180), xMovement, yMovement, zMovement, 0, 1, 0);
			break;
	}

	// Set submarine material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

	// Apply transformations to construct submarine, modify this!
	glPushMatrix();
	glTranslatef(xMovement, yMovement, zMovement);
		glPushMatrix();
		glRotatef(yRotation, 0, 1, 0);

			myDrawShip();
	
		glPopMatrix();
	glPopMatrix();

	//Draws trees
	float range = 12;
	float distanceBetween = 8;
	glPushMatrix();
	glScalef(1, 6, 1);
	for (int i = 0; i < (range*2/distanceBetween + 1); i++) {
		glPushMatrix();
		glTranslatef(-range, 0, -range + (i * distanceBetween));
		glutSolidCube(1);
		for (int i = 0; i < (range * 2 / distanceBetween + 1); i++) {
			glTranslatef(distanceBetween, 0, 0);
			glutSolidCube(1);
		}
		glPopMatrix();
	}

	glPopMatrix();

	// Draw ground/sea floor
	glPushMatrix();
		glTranslatef(0, -1.5, 0);
		glScalef(4, 1, 4);
		DrawMeshQM(&groundMesh, meshSize);
	glPopMatrix();


	glutSwapBuffers();   // Double buffering, swap buffers
}

void myDrawShip() {
	//Body
	glPushMatrix();
	glRotatef(0, 0, 0, 0);
	glTranslatef(0, 0, 0);
	glScalef(2.5, 1, 1);
	glutSolidSphere(1, 20, 20);
	glPopMatrix();

	//Nose
	glPushMatrix();
	glTranslatef(-2.475, 0, 0);
	glRotatef(90, 0, -90, 0);
	glScalef(0.1, 0.1, 0.1);
	glutSolidCone(1, 2, 20, 20);
	glPopMatrix();

	//Cabin
	glPushMatrix();
	glRotatef(0, 0, 0, 0);
	glTranslatef(0, -1.0625, 0);
	glScalef(1, 0.25, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	//Vertical wing rear
	glPushMatrix();
	glRotatef(0, 0, 0, 0);
	glTranslatef(2, 0, 0);
	glScalef(0.5, 2, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//Vertical wing close
	glPushMatrix();
	glRotatef(0, 0, 0, 0);
	glTranslatef(1.5, 0, 0);
	glScalef(0.5, 2, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//Horizontal wing rear
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(2, 0, 0);
	glScalef(0.5, 2, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//Horizontal wing close
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(1.5, 0, 0);
	glScalef(0.5, 2, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//Engine rod
	glPushMatrix();
	glTranslatef(0.45, -1.2, 0);
	glRotatef(0, 0, 0, 0);
	glScalef(0.1, 0.025, 1.0);
	glutSolidCube(1);
	glPopMatrix();

	//Left Engine
	//Engine main
	glPushMatrix();
	glTranslatef(0.25, -1.2, 0.45);
	glRotatef(90, 0, 1, 0);
	glScalef(0.1, 0.1, 0.3);
	GLUquadricObj *cyl;
	cyl = gluNewQuadric();
	mySolidCylinder(cyl, 1, 1, 1, 20, 20);
	glPopMatrix();

	//Engine nose
	glPushMatrix();
	glTranslatef(0.25, -1.2, 0.45);
	glRotatef(-90, 0, 1, 0);
	glScalef(0.1, 0.1, 0.1);
	glutSolidCone(1, 1, 20, 20);
	glPopMatrix();

	//Engine rod
	glPushMatrix();
	glTranslatef(0.575, -1.2, 0.45);
	glRotatef(90, 0, 0, 1);
	glScalef(0.01, 0.1, 0.01);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller base
	glPushMatrix();
	glTranslatef(0.625, -1.2, 0.45);
	glRotatef(90, 0, 0, 1);
	glScalef(0.025, 0.025, 0.025);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller 1
	glPushMatrix();
	glTranslatef(0.625, -1.2, 0.45);
	glRotatef(0 + propellorRotation, 1, 0, 0);
	glScalef(0.01, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller 2
	glPushMatrix();
	glTranslatef(0.625, -1.2, 0.45);
	glRotatef(90 + propellorRotation, 1, 0, 0);
	glScalef(0.01, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	//left Engine
	//Engine main
	glPushMatrix();
	glTranslatef(0.25, -1.2, -0.45);
	glRotatef(90, 0, 1, 0);
	glScalef(0.1, 0.1, 0.3);
	GLUquadricObj *cyl2;
	cyl2 = gluNewQuadric();
	mySolidCylinder(cyl2, 1, 1, 1, 20, 20);
	glPopMatrix();

	//Engine nose
	glPushMatrix();
	glTranslatef(0.25, -1.2, -0.45);
	glRotatef(-90, 0, 1, 0);
	glScalef(0.1, 0.1, 0.1);
	glutSolidCone(1, 1, 20, 20);
	glPopMatrix();

	//Engine rod
	glPushMatrix();
	glTranslatef(0.575, -1.2, -0.45);
	glRotatef(90, 0, 0, 1);
	glScalef(0.02, 0.1, 0.02);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller base
	glPushMatrix();
	glTranslatef(0.625, -1.2, -0.45);
	glRotatef(90, 0, 0, 1);
	glScalef(0.025, 0.025, 0.025);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller 1
	glPushMatrix();
	glTranslatef(0.625, -1.2, -0.45);
	glRotatef(0 + propellorRotation, 1, 0, 0);
	glScalef(0.01, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	//Engine propeller 2
	glPushMatrix();
	glTranslatef(0.625, -1.2, -0.45);
	glRotatef(90 + propellorRotation, 1, 0, 0);
	glScalef(0.01, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	//printf("");
	switch (key)
	{
	case 'r':
		xMovement = 0;
		yMovement = 0;
		zMovement = 0;
		yRotation = 0;
		propellorRotation = 0;
		velocity = 0;
		break;
	case 'i':
		yMovement += 0.5;
		break;
	case 'k':
		yMovement -= (yMovement > 0) ? 0.5 : 0;
		break;
	}
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		printf("Controls \n");
		printf("-------------\n");
		printf("Up Arrow	: Increment movement speed in positive direction.(speed up and forward)\n");
		printf("Down Arrow	: Increment movement speed in negative direction.(slow down and reverse)\n");
		printf("Left Arrow	: Rotate your ship to the left.\n");
		printf("Right Arrow	: Rotate your ship to the right.\n");
		printf("I Key		: Move your ship up.\n");
		printf("K Key		: Move your ship down.\n");
		printf("F2		: Top down view(look at origin from 0,20,0)\n");
		printf("F3		: Side view(look at origin from 0,0,20)\n");
		printf("F4		: Lock on behind view.\n");
		printf("R Key		: Reset ship(recenters and stops).\n");

	}
	else if (key == GLUT_KEY_F2)
	{
		view = 0;
	}
	else if (key == GLUT_KEY_F3)
	{
		view = 1;
	}
	else if (key == GLUT_KEY_F4)
	{
		view = 2;
	}
	else if (key == GLUT_KEY_UP) {
		velocity += shipSpeed;
	}
	else if (key == GLUT_KEY_DOWN) {
		velocity -= shipSpeed;
	}
	else if (key == GLUT_KEY_LEFT) {
		yRotation += 2;
	}
	else if (key == GLUT_KEY_RIGHT) {
		yRotation -= 2;
	}
}



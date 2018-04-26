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
#include "Matrix3D.h"

const int meshSize = 64;    // Default Mesh Size
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
static GLfloat default_ambient[] = { 68.0F / 255.0F, 143.0F / 255.0F, 163.0F / 255.0F, 1.0F };//Blueish color
static GLfloat black_ambient[] = { 0.0F / 255.0F, 0.0F / 255.0F, 0.0F / 255.0F, 1.0F };//Blueish color
static GLfloat default_specular[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat default_diffuse[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat default_shininess[] = { 0.0F };

// A quad mesh representing the ground / sea floor 
static QuadMesh groundMesh;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void calculateShovelPoint();
void collideMesh(QuadMesh* qm, float colPosX, float colPosZ, float depth);
void checkCollision(Matrix3D *m);void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);

//My variables
float boomLength = 3;
float armLength = 2.5;
float shovelLength = .625;

float rotateY = 0;
float rotateZ1 = 0;
float rotateZ2 = 0;

float camDistance = 15;

float mouseStartPosX;
float mouseStartPosY;
float mouseChangeX = -180;
float mouseChangeY = -120;

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 2");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
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

	Vector3D ambient = NewVector3D(185.0F / 255.0F, 122.0F / 255.0F, 87.0F / 255.0F);
	Vector3D diffuse = NewVector3D(0.4f, 0.4f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	Matrix3D m = NewIdentity();
	MatrixLeftMultiplyV(&m, NewTranslate(0, camDistance, 0));//Boom
	MatrixLeftMultiplyV(&m, NewRotateZ(-mouseChangeY / 2));//Boom rotation on z
	MatrixLeftMultiplyV(&m, NewRotateY(-mouseChangeX / 2));//Boom rotation on y

	/*float camPosX = camDistance * sinf(-mouseChangeX / 4 * 3.1415926535 / 180);// -camDistance *  cosf(-mouseChangeY / 4 * 3.1415926535 / 180);
	float camPosY = 12.0 / 12.0 * camDistance;
	float camPosZ = camDistance * cosf(-mouseChangeX / 4 * 3.1415926535 / 180);// +camDistance *  sinf(-mouseChangeY / 4 * 3.1415926535 / 180);*/
	float camPosX = m.matrix[0][3];
	float camPosY = m.matrix[1][3];
	float camPosZ = m.matrix[2][3];
	gluLookAt(camPosX, camPosY, camPosZ, 0, 0, 0, 0, 1, 0);

	// Set submarine material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, default_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, default_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, default_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, default_shininess);

	//VIEW WIREFRAME
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Joint1
	glPushMatrix();
		glTranslatef(0, .75, 0);
		glutSolidSphere(.5, 20, 20);//Base sphere

		glRotatef(rotateY, 0, 1, 0);
		glRotatef(rotateZ1, 0, 0, 1);
		glTranslatef(0, boomLength/2, 0);

		glPushMatrix();
			glTranslatef(0, boomLength/2, 0);
			glRotatef(rotateZ2, 0, 0, 1);
			glTranslatef(0, armLength/2, 0);
			glScalef(.4, armLength, .4);//Arm scale
			glutSolidCube(1);//Arm

			glTranslatef(.25, .5, 0);
			glScalef(.5, .5, .5);//Shovel scale is half of arm scale
			glutSolidCube(1);//Shovel
		glPopMatrix();

		glScalef(.5, boomLength, .5);//Boom scale
		glutSolidCube(1);//Boom
	glPopMatrix();

	//Black base box
	glMaterialfv(GL_FRONT, GL_AMBIENT, black_ambient);
	glPushMatrix();
		glTranslatef(0, .25, 0);
		glScalef(.5, .5, .5);
		glutSolidCube(1);
	glPopMatrix();

	calculateShovelPoint();//Calculate the shovel point and checks for collisions

	ComputeNormalsQM(&groundMesh);
	DrawMeshQM(&groundMesh, meshSize);

	glutSwapBuffers();   // Double buffering, swap buffers
}

void calculateShovelPoint() {
	Matrix3D m = NewIdentity();
	MatrixLeftMultiplyV(&m, NewTranslate(0.2, 0.625, 0));//Shovel
	MatrixLeftMultiplyV(&m, NewTranslate(0, armLength, 0));//Arm
	MatrixLeftMultiplyV(&m, NewRotateZ(rotateZ2));//ArmRotation
	MatrixLeftMultiplyV(&m, NewTranslate(0, boomLength, 0));//Boom
	MatrixLeftMultiplyV(&m, NewRotateZ(rotateZ1));//Boom rotation on z
	MatrixLeftMultiplyV(&m, NewRotateY(rotateY));//Boom rotation on y
	MatrixLeftMultiplyV(&m, NewTranslate(0, 0.75, 0));//OffsetFrom ground

	//MatrixPrint(&m);
	checkCollision(&m);
}

void checkCollision(Matrix3D *m) {
	//if (m->matrix[1][3] <= 0) {
		//printf("H: %f \t", m->matrix[1][3]);
		//printf("xPos: %f \t", m->matrix[0][3]);
		//printf("zPos: %f\n", m->matrix[2][3]);

		collideMesh(&groundMesh, m->matrix[0][3], m->matrix[2][3], m->matrix[1][3]);
	//}
}

//Deforms mesh using basic exponential math, sort of imitiating the gaussian
void collideMesh(QuadMesh* qm, float colPosX, float colPosZ, float depth) {
	int i;
	int j;
	float radius = (-depth);
	for (i = 0; i < meshSize * meshSize; i++) {
		for (j = 0; j < 4; j++) {
			float xDis = qm->quads[i].vertices[j]->position.x - colPosX;
			float zDis = qm->quads[i].vertices[j]->position.z - colPosZ;
			float distance = sqrtf(zDis*zDis + xDis*xDis);
			//printf("%f \n", distance);
			//When distance = 0; y pos = depth
			//when distance = 8; y pos = 0
			//(Distance - radius) / radius gives a value from 0 - 1 
			//float loweringVal = depth * ((distance - radius) / radius);
			if (distance < radius) {
				float loweringVal;
				float magicScale = (distance - radius) / radius; //value from 0to-1. 0 being furthest, -1 being closest. Eg. (0 - radius) / radius = -1
				int magicScaler = 2;
				magicScale *= magicScaler;
				if (distance < radius / 2) {
					//loweringVal = (1 - pow(((distance - radius / 2) / radius / 2), 2)) * depth;
					//loweringVal = ((radius / 2) - pow(((distance - radius / 2) / radius / 2), 2)) * depth;
					//we want the -2 to -1 be represented as -1 to 0 and then as 0 to -1
					magicScale = -(magicScale + magicScaler);
					loweringVal = (pow(magicScale, 2) * -depth / pow(2, magicScaler - 1)) + (pow(-0.5 * magicScaler, 2) * depth / pow(2, magicScaler - 1) * 2);
				}
				else { //Magic val is -1 to 0
					loweringVal = pow(magicScale, 2) * depth / pow(2, magicScaler - 1);
				}
				if ((loweringVal < qm->quads[i].vertices[j]->position.y)) {
					qm->quads[i].vertices[j]->position.y = loweringVal;
				}
			}
		}
	}
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
	float moveSpeed = 1;
	switch (key)
	{
	case 'a':
		rotateY += moveSpeed;
		break;
	case 'd':
		rotateY -= moveSpeed;
		break;
	case 'w':
		rotateZ1 += moveSpeed;
		if (rotateZ1 > 0) {
			rotateZ1 = 0;
		}
		break;
	case 's':
		rotateZ1 -= moveSpeed;
		if (rotateZ1 < -70) {
			rotateZ1 = -70;
		}
		break;
	case 'r':
		rotateZ2 += moveSpeed;
		if (rotateZ2 > 0) {
			rotateZ2 = 0;
		}
		break;
	case 'f':
		rotateZ2 -= moveSpeed;
		if (rotateZ2 < -110) {
			rotateZ2 = -110;
		}
		break;
	case 't':
		boomLength += moveSpeed / 10;
		if (boomLength > 3.5) {
			boomLength = 3.5;
		}
		break;
	case 'g':
		boomLength -= moveSpeed / 10;
		if (boomLength < 2.5) {
			boomLength = 2.5;
		}
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1){
		printf("Controls \n");
		printf("-------------\n");
		printf("W Key	: Tilt boom up.\n");
		printf("S Key	: Tilt boom down.\n");
		printf("A Key	: Rotate boom counter clock-wise.\n");
		printf("D Key	: Rotate boom clock-wise.\n");
		printf("R Key	: Tilt arm up.\n");
		printf("F Key	: Tilt arm dow.\n");
		printf("T Key	: Extend boom length.\n");
		printf("G Key	: Shrink boom length.\n");
		printf("Click and drag to move camera.\n");
		printf("Scroll to zoom.");
	}
	glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN){
			mouseStartPosX = x - mouseChangeX;
			mouseStartPosY = y - mouseChangeY;
		}
		break;
	case 3:
		camDistance -= .25;
		if (camDistance < 3) {
			camDistance = 3;
		}
		break;
	case 4:
		camDistance += .25;
		if (camDistance > 20) {
			camDistance = 20;
		}
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		mouseChangeX = xMouse - mouseStartPosX;
		mouseChangeY = yMouse - mouseStartPosY;
		//printf("%f\n", mouseChangeY);
		if (mouseChangeY < -359){
			mouseChangeY = -359;
		}
		else if (mouseChangeY > -1) {
			mouseChangeY = -1;
		}
	}

	glutPostRedisplay();   // Trigger a window redisplay
}



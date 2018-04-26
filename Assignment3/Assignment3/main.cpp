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
#include "RGBpixmap.h"

#define PI 3.1415926534

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
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void calculateHeight(struct transform *p);
void drawPlayer(struct transform p);
void movePlayer(struct transform *p);
void playDieAnim(struct transform *p);
bool checkCollision(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float range);
float distance(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z);
void readTextures();
void readTexture(int i, string name);

void update(int x);
void collideMesh(QuadMesh* qm, float colPosX, float colPosZ, float depth, int radiusScale);

void mySolidSphere(float radius, float slices, float stacks);

//My variables


struct transform {
	Vector3D position;
	Vector3D rotation;
	Vector3D scale;
	float moveSpeed;
	float rotationSpeed;
	int health = 100;
	bool alive = true;
};

float camDistance = 14;

float mouseStartPosX;
float mouseStartPosY;
float mouseChangeX = -180;
float mouseChangeY = -120;

struct transform player1;
struct transform player2;

float moveSpeedIncrement = 0.01f;

float rotateIncrement = 1.0f;

int hitCounter = 200;
bool firstPerson = false;

RGBpixmap pix[4];         // Data structure containing texture read from a file
GLuint textureId[4];

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
	glutTimerFunc(1000 / 60, update, 0);
	glutMotionFunc(mouseMotionHandler);
	glutSpecialFunc(functionKeys);

	collideMesh(&groundMesh, 4.125407f, 4.125407f, -0.930707f, 10);

	player1.position.x = -3.5;
	player1.position.y = 0;
	player1.position.z = -3.5;
	player1.rotation.y = 45;

	player2.position.x = 3.5;
	player2.position.y = 0;
	player2.position.z = 3.5;
	player2.rotation.y = 225;
	
	readTextures();
	printf("Press F1 to see controls.\n");
	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

//Deforms mesh using basic exponential math, sort of imitiating the gaussian
void collideMesh(QuadMesh* qm, float colPosX, float colPosZ, float depth, int radiusScale) {
	//printf("%f\n %f\n %f\n", colPosX, colPosZ, depth);
	int i;
	int j;
	float radius = (-depth) * radiusScale;
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
				qm->quads[i].vertices[j]->position.y = loweringVal;
			}
			else { //Magic val is -1 to 0
				loweringVal = pow(magicScale, 2) * depth / pow(2, magicScaler - 1);
				qm->quads[i].vertices[j]->position.y = loweringVal;
			}
		}
	}
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

	glClearColor(0.6f, 0.6f, 0.6f, 0.0);  // Background color
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);              // Apply a texture map. There is no lighting/shading.

	glMatrixMode(GL_MODELVIEW);
}

void readTextures() {
	readTexture(0, "floor.bmp");
	readTexture(1, "rubber.bmp");
	readTexture(2, "metal.bmp");
}

void readTexture(int i, string name) {
	pix[i].readBMPFile(name);
	glGenTextures(1, &textureId[i]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, textureId[i]);   // store pixels by byte
	glBindTexture(GL_TEXTURE_2D, textureId[i]); // select current texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(      // initialize texture
		GL_TEXTURE_2D, // texture is 2-d
		0,             // resolution level 0
		GL_RGB,        // internal format
		pix[i].nCols,    // image width
		pix[i].nRows,    // image height
		0,             // no border
		GL_RGB,        // my format
		GL_UNSIGNED_BYTE, // my type
		pix[i].pixel);   // the pixels
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	if (player1.alive) {
		calculateHeight(&player1);
	}
	if (player2.alive) {
		calculateHeight(&player2);
	}

	if (!firstPerson) {
		Matrix3D m = NewIdentity();
		MatrixLeftMultiplyV(&m, NewTranslate(0, camDistance, 0));
		MatrixLeftMultiplyV(&m, NewRotateZ(-mouseChangeY / 2));
		MatrixLeftMultiplyV(&m, NewRotateY(-mouseChangeX / 2));

		float camPosX = m.matrix[0][3];
		float camPosY = m.matrix[1][3];
		float camPosZ = m.matrix[2][3];
		gluLookAt(camPosX, camPosY, camPosZ, 0, 0, 0, 0, 1, 0);
	}
	else {
		Matrix3D camPos = NewIdentity();
		MatrixLeftMultiplyV(&camPos, NewTranslate(player1.position.x, player1.position.y + 1.4, player1.position.z));
		MatrixRightMultiplyV(&camPos, NewRotateY(player1.rotation.y));
		MatrixRightMultiplyV(&camPos, NewTranslate(0, 0, -0.2));

		Matrix3D camLookAt = NewIdentity();
		MatrixRightMultiplyV(&camLookAt, NewTranslate(player1.position.x, player1.position.y + 1.2, player1.position.z));
		MatrixRightMultiplyV(&camLookAt, NewRotateY(player1.rotation.y));
		MatrixRightMultiplyV(&camLookAt, NewRotateX(20.0));
		MatrixRightMultiplyV(&camLookAt, NewTranslate(0, 0, 1));

		gluLookAt(camPos.matrix[0][3], camPos.matrix[1][3], camPos.matrix[2][3], camLookAt.matrix[0][3], camLookAt.matrix[1][3], camLookAt.matrix[2][3], 0, 1, 0);
	}

	//VIEW WIREFRAME
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	drawPlayer(player1);
	drawPlayer(player2);

	glBindTexture(GL_TEXTURE_2D, 1);
	ComputeNormalsQM(&groundMesh);
	DrawMeshQM(&groundMesh, meshSize);

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawPlayer(struct transform p) {
	glPushMatrix();
	glTranslatef(p.position.x, p.position.y + 0.5f + 0.1f, p.position.z);

	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	static GLfloat red_ambient[] = { 1.0F, 0.0F, 0.0F, 1.0F };
	static GLfloat green_ambient[] = { 0.0F, 1.0F, 0.0F, 1.0F };
	static GLfloat specular[] = { 0.0F, 0.0F, 0.0F, 1.0F };
	static GLfloat diffuse[] = { 0.0F, 0.0F, 0.0F, 0.0F };
	static GLfloat shininess[] = { 0.0F };

	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	float healthVal = p.health / 100.0;
	glTranslatef(0, 0.6, 0);
	if (!firstPerson) {
		glRotatef(-mouseChangeX / 2.0 - 90, 0, 1, 0);
	}
	else {
		glRotatef(p.rotation.y, 0, 1, 0);
	}

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, green_ambient);
	if (firstPerson) {
		glTranslatef(0, 0.2, 0.5);
	}
	glTranslatef(+(0.5 - healthVal / 2), 0.0, 0.0);
	glScalef((p.health / 100.0), 0.05, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, red_ambient);
	if (firstPerson) {
		glTranslatef(0, 0.2, 0.5);
	}
	glTranslatef(-0.5 + (0.5 - healthVal / 2), 0.0, 0.0);
	glScalef((1 - p.health / 100.0), 0.05, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glRotatef(p.rotation.x, 1, 0, 0);
	glRotatef(p.rotation.y, 0, 1, 0);
	glRotatef(p.rotation.z, 0, 0, 1);
	glPushMatrix();
	float radiusScale = .25;
	glBindTexture(GL_TEXTURE_2D, 2);
	glTranslatef(0.5f * radiusScale, -.5f, .5f * radiusScale); //Bottom right(front left)
	mySolidSphere(.1, 10, 10);
	glTranslatef(-1.0 * radiusScale, 0.0f, 0.0f);//Bottom left(front right)
	mySolidSphere(.1, 10, 10);
	glTranslatef(0.0f, 0.0f, -1.0f * radiusScale);//top left(back right)
	mySolidSphere(.1, 10, 10);
	glTranslatef(1.0f * radiusScale, 0.0f, 0.0f);//top right(back left)
	mySolidSphere(.1, 10, 10);
	glTranslatef(-.5f * radiusScale, 0.5f, .5f + 0.5 * radiusScale);
	glScalef(.1f, .1f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 3);
	glutSolidCube(1.0f);
	glPopMatrix();
	mySolidSphere(.5, 10, 10);
	glPopMatrix();
}

void mySolidSphere(float radius, float slices, float stacks) {
	GLUquadricObj *mySphere;
	mySphere = gluNewQuadric();
	gluQuadricDrawStyle(mySphere, GLU_FILL);
	gluQuadricTexture(mySphere, TRUE);
	gluQuadricNormals(mySphere, GLU_SMOOTH);
	gluSphere(mySphere, radius, slices, stacks);
	gluDeleteQuadric(mySphere);
}

void movePlayer(struct transform *p) {
	p->rotation.y += p->rotationSpeed;

	if (p->rotationSpeed > 0) {
		if (p->rotationSpeed - rotateIncrement / 20 <= 0) {
			p->rotationSpeed = 0;
		}
		else {
			p->rotationSpeed -= rotateIncrement / 20;
		}
	}
	else if (p->rotationSpeed < 0) {
		if (p->rotationSpeed + rotateIncrement / 20 >= 0) {
			p->rotationSpeed = 0;
		}
		else {
			p->rotationSpeed += rotateIncrement / 20;
		}
	}

	if (p->position.x < 7.5 && p->position.x > -7.5) {
		p->position.x += sin(p->rotation.y * PI / 180.0f) * p->moveSpeed;
	}
	else {
		p->moveSpeed *= -1;
		p->position.x += sin(p->rotation.y * PI / 180.0f) * p->moveSpeed;
	}
	if (p->position.z < 7.5 && p->position.z > -7.5) {
		p->position.z += cos(p->rotation.y * PI / 180.0f) * p->moveSpeed;
	}
	else {
		p->moveSpeed *= -1;
		p->position.z += cos(p->rotation.y * PI / 180.0f) * p->moveSpeed;
	}

	if (p->position.x > 8 || p->position.x < -8 || p->position.z > 8 || p->position.z < -8) {
		p->position.x = 0;
		p->position.z = 0;
	}

	if (p->moveSpeed > 0) {
		if (p->moveSpeed - moveSpeedIncrement / 60 < 0) {
			p->moveSpeed = 0;
		}
		else {
			p->moveSpeed = p->moveSpeed - moveSpeedIncrement / 60;
		}
	}
	else if (p->moveSpeed < 0) {
		if (p->moveSpeed + moveSpeedIncrement / 60 > 0) {
			p->moveSpeed = 0;
		}
		else {
			p->moveSpeed = p->moveSpeed + moveSpeedIncrement / 60;
		}
	}
}

void playDieAnim(struct transform *p) {
	p->rotationSpeed += 0.25f;
	p->position.y += 0.02f;
	movePlayer(p);
}

void update(int x) {
	if (!player1.alive) {
		//printf("ded1");
		playDieAnim(&player1);
	}
	if (!player2.alive) {
		playDieAnim(&player2);
	}
	else if (player1.alive && player2.alive){
		float targetRotation = player1.rotation.y + player1.rotationSpeed;

		float targetPosX = player1.position.x + sin(player1.rotation.y * PI / 180.0f) * player1.moveSpeed;
		float targetPosY = player1.position.y; //Not exact cause of terrain but who cares
		float targetPosZ = player1.position.z + cos(player1.rotation.y * PI / 180.0f) * player1.moveSpeed;

		Matrix3D nose1 = NewIdentity();
		MatrixRightMultiplyV(&nose1, NewTranslate(targetPosX, player1.position.y, targetPosZ));
		MatrixRightMultiplyV(&nose1, NewRotateY(targetRotation));//ArmRotation
		MatrixRightMultiplyV(&nose1, NewTranslate(0, 0, 1));

		Matrix3D nose2 = NewIdentity();
		MatrixRightMultiplyV(&nose2, NewTranslate(player2.position.x, player2.position.y, player2.position.z));
		MatrixRightMultiplyV(&nose2, NewRotateY(player2.rotation.y));//ArmRotation
		MatrixRightMultiplyV(&nose2, NewTranslate(0, 0, 1));

		bool ponp = checkCollision(targetPosX, targetPosY, targetPosZ, player2.position.x, player2.position.y, player2.position.z, 1 - .05); //Player on player
		bool p1onNose = checkCollision(targetPosX, targetPosY, targetPosZ, nose2.matrix[0][3], nose2.matrix[1][3], nose2.matrix[2][3], .5 - .05); //player 1 on nose 2
		bool p2onNose = checkCollision(player2.position.x, player2.position.y, player2.position.z, nose1.matrix[0][3], nose1.matrix[1][3], nose1.matrix[2][3], .5 - .05); //player 2 on nose 1
		bool nonn = checkCollision(nose1.matrix[0][3], nose1.matrix[1][3], nose1.matrix[2][3], nose2.matrix[0][3], nose2.matrix[1][3], nose2.matrix[2][3], .1 - .05); //nose 1 on nose 2

		if (!p1onNose && !ponp && !p2onNose && !nonn) {
			hitCounter++;
			if (hitCounter >= 240) {
				player2.moveSpeed = .02;
				//player2.rotationSpeed = rand() / 10;
				if (hitCounter % 240 == 0) {
					float xDis = player2.position.x - player1.position.x;
					float zDis = player2.position.z - player1.position.z;
					float yRotation = atanf(xDis / zDis) * 180 / PI + 180;
					if (player1.position.z > player2.position.z) {
						yRotation += 180;
					}
					player2.rotation.y = yRotation;
				}
			}
		}
		else {
			if (hitCounter > 240) {
				if (p1onNose) {
					player1.health -= 20;
				}
				if (p2onNose) {
					player2.health -= 20;
				}
				else if (nonn) {
					player1.health -= 20;
					player2.health -= 20;
				}
				else if (ponp) {
					player1.health -= 10;
					player2.health -= 10;
				}
				if (player1.health <= 0) {
					player1.health = 0;
					player1.alive = false;
				}
				if (player2.health <= 0) {
					player2.health = 0;
					player2.alive = false;
				}
			}
			hitCounter = 120;
			player1.moveSpeed = player1.moveSpeed > 0 ? -0.03 : 0.02;
			player2.moveSpeed = -.03;
		}
		movePlayer(&player1);
		movePlayer(&player2);
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, update, 0);
}

float distance(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z) {
	float xDistance = p1x - p2x;
	float yDistance = p1y - p2y;
	float zDistance = p1z - p2z;

	return  sqrt(pow(xDistance, 2) + pow(yDistance, 2) + pow(zDistance, 2));
}

bool checkCollision(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float range) {
	float baseDistance = distance(p1x, p1y, p1z, p2x, p2y, p2z);
	if (baseDistance <= range) {
		/*player1.moveSpeed = 0;
		player2.moveSpeed = 0;
		player1.rotationSpeed = 0;
		player2.rotationSpeed = 0;*/
		//printf("Crashed: %f\nRange: %f\n", baseDistance, range);
		return true;
	}
	return false;
}

void calculateHeight(struct transform *p) {
	float xPos = p->position.x + 8;
	float zPos = 8 - p->position.z;

	int gridX = (int)(xPos / 16.0 * 64.0);
	int gridZ = (int)(zPos / 16.0 * 64.0);

	if (gridX < 0 || gridX > 63 || gridZ < 0 || gridZ > 63) {
		return;
	}

	//printf("<%i><%i>\n", gridX, gridZ);
	//printf("<%f><%f>\n", p->position.x, p->position.z);
	int quad = gridZ * 64 + gridX;

	float closestDistance = 1000.0;
	float closestHeight = 0.0;
	for (int i = 0; i < 4; i++) {
		float xDistance = p->position.x - groundMesh.quads[quad].vertices[i]->position.y;
		float yDistance = p->position.z - groundMesh.quads[quad].vertices[i]->position.z;
		float distance = sqrt(pow(xDistance, 2) + pow(yDistance, 2));
		if (distance < closestDistance) {
			closestDistance = distance;
			closestHeight = groundMesh.quads[quad].vertices[i]->position.y;
		}
	}
	p->position.y = closestHeight;

	float xRotation =
		(groundMesh.quads[quad].vertices[0]->position.y
			+ groundMesh.quads[quad].vertices[1]->position.y) / 2
		-
		(groundMesh.quads[quad].vertices[2]->position.y
			+ groundMesh.quads[quad].vertices[3]->position.y) / 2;

	//printf("<%f> ", xRotation);
	xRotation = atan(xRotation) * 180 / PI;
	//printf("<%f>\n", xRotation);

	float zRotation =
		(groundMesh.quads[quad].vertices[0]->position.y
			+ groundMesh.quads[quad].vertices[3]->position.y) / 2
		-
		(groundMesh.quads[quad].vertices[1]->position.y
			+ groundMesh.quads[quad].vertices[2]->position.y) / 2;

	zRotation = atan(zRotation) * 180 / PI;

	//p->rotation.x = xRotation;
	//p->rotation.z = zRotation;
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
	switch (key)
	{
	case 'a':
		player1.rotationSpeed += rotateIncrement;
		if (player1.rotationSpeed > 2) player1.rotationSpeed = 2;
		break;
	case 'd':
		player1.rotationSpeed -= rotateIncrement;
		if (player1.rotationSpeed < -2) player1.rotationSpeed = -2;
		break;
	case 'w':
		player1.moveSpeed += moveSpeedIncrement;
		if (player1.moveSpeed > 10) player1.moveSpeed = 10;
		//printf("Move: %f\n", player1.moveSpeed);
		break;
	case 's':
		player1.moveSpeed -= moveSpeedIncrement;
		if (player1.moveSpeed < -10) player1.moveSpeed = -10;
		break;
	case 'f':
		firstPerson = !firstPerson;
		break;
	case 'r':
		player1.position.x = -3.5;
		player1.position.y = 0;
		player1.position.z = -3.5;
		player1.rotation.y = 45;
		player1.rotationSpeed = 0;
		player1.moveSpeed = 0;
		player1.alive = true;
		player1.health = 100;

		player2.position.x = 3.5;
		player2.position.y = 0;
		player2.position.z = 3.5;
		player2.rotation.y = 225;
		player2.rotationSpeed = 0;
		player2.moveSpeed = 0;
		player2.alive = true;
		player2.health = 100;

		hitCounter = 200;

		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1) {
		printf("Controls \n");
		printf("-------------\n");
		printf("W Key	: Accelerate.\n");
		printf("S Key	: Decelerate.\n");
		printf("A Key	: Turn Left.\n");
		printf("D Key	: Turn Right.\n");
		printf("R Key	: Reset.\n");
		printf("F Key	: Toggle First Person.\n");
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
		if (state == GLUT_DOWN) {
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
		if (mouseChangeY < -359) {
			mouseChangeY = -359;
		}
		else if (mouseChangeY > -1) {
			mouseChangeY = -1;
		}
	}

	glutPostRedisplay();   // Trigger a window redisplay
}



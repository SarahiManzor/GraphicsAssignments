//#include <stdio.h>
//#include <math.h>
//#include <GL/glut.h>
//#include "RGBpixmap.h"
//
//#ifndef M_PI
//#define M_PI 3.14159265358979323846
//#endif
//
//RGBpixmap pix1;         // Data structure containing texture read from a file
//GLuint textureId;
//const int delang = 5;           // Try increasing this to 60
//double yang = 0.0;
//double xang = 0.0;
//
//
//void display()
//{
//    const double r0 = 0.22f;   // Cylinder radius
//    int ang;
//    double x0 = r0;
//    double x1 = r0;
//    double z0 = 0;
//    double z1 = 0;
//
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glLoadIdentity();
//    glRotated(yang, 1.0, 1.0, 0.0);
//    glTranslated(-0.4, 0.25, 0.0);
//    glRotated(xang, 0.0, 0.0, 1.0);
//
//    glBegin(GL_QUADS);
//    for (ang = 0; ang <= 360; ang += delang)
//    {
//        x0 = r0*cos((double)ang*2.0*M_PI / 360.0);
//        x1 = r0*cos((double)(ang + delang)*2.0*M_PI / 360.0);
//        z0 = r0*sin((double)ang*2.0*M_PI / 360.0);
//        z1 = r0*sin((double)(ang + delang)*2.0*M_PI / 360.0);
//
//        glTexCoord2d(ang / 360.0, 0.0);
//        glVertex3d(x0, 0.0, z0);
//
//        glTexCoord2d(ang / 360.0, 1.0);
//        glVertex3d(x0, 0.8, z0);           // y value controls cylinder height
//
//        glTexCoord2d((ang + delang) / 360.0, 1.0);
//        glVertex3d(x1, 0.8, z1);
//
//        glTexCoord2d((ang + delang) / 360.0, 0.0);
//        glVertex3d(x1, 0.0, z1);
//
//        x0 = x1;
//        z0 = z1;
//    }
//    glEnd();
//
//	yang = yang + 1.35;
//    if (yang > 360.0) {
//        yang = 0.0;
//    }
//	xang = xang - 1.15;
//    if (xang < 0.0) {
//        xang = 360.0;
//    }
//
//    glutSwapBuffers();
//}
//
//void init()
//{
//    glClearColor(0.3f, 0.1f, 0.2f, 0.0);  // Background color
//    glClearDepth(1.0);
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_TEXTURE_2D);              // Apply a texture map. There is no lighting/shading.
//
//    pix1.readBMPFile("mandrill.bmp");
//    glGenTextures(1, &textureId);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // store pixels by byte	
//    glBindTexture(GL_TEXTURE_2D, textureId); // select current texture
//    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexImage2D(      // initialize texture
//        GL_TEXTURE_2D, // texture is 2-d
//        0,             // resolution level 0
//        GL_RGB,        // internal format
//        pix1.nCols,    // image width
//        pix1.nRows,    // image height
//        0,             // no border
//        GL_RGB,        // my format
//        GL_UNSIGNED_BYTE, // my type
//        pix1.pixel);   // the pixels
//
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-1, .8, -1, .8, -2, 2);
//
//    glMatrixMode(GL_MODELVIEW);
//}
//
//void animationHandler(int param)
//{
//    glutPostRedisplay();
//    glutTimerFunc(20, animationHandler, 0);    // Restart timer
//}
//
//void main(int argc, char** argv)
//{
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
//    glutInitWindowSize(600, 600);
//    glutInitWindowPosition(0, 0);
//    glutCreateWindow("Canned Monkey");
//    glutDisplayFunc(display);
//    init();
//    glutTimerFunc(20/*msec*/, animationHandler, 0);   // Display a new frame every 20 ms
//
//    glutMainLoop();
//}
//

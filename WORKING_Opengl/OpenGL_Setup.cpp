#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

void display() {
	// Set color of background (def=black) to Yellow
	glClearColor(0, 1, 1, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);  //or GL_PROJECTION) if you want to affect the projection mat
	glLoadIdentity();//has to be at beg of any prog that has transformations
	glLineWidth(2);
	//Axes (Blue)
	glColor3f(0.0, 0, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glEnd();
	glFlush();
}


int main(int argc, char* argv[]) {

	// Initialize GLUT
	glutInit(&argc, argv);

	// Set the window size
	glutInitWindowSize(800, 600);
	// Create the window with the title "Hello,GL"
	glutCreateWindow("Hello, GL");
	// Bind the two functions (above) to respond when necessary
	glutDisplayFunc(display);

	// To avoid window to be closed
	glutMainLoop();
	return 0;
}
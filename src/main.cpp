#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GLEW_STATIC

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

GLuint loadTexture(const char* filePath) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0); //IMAGE STB TEST!!!!
    if (!data) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return 0;
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitWindowSize(800, 600); // Set the window size
    glutCreateWindow("Hello, GL"); // Create the window

    glewExperimental = GL_TRUE;  // Enable modern OpenGL features (optional)
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        return -1;
    }

    //GLM TEST
    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 movement(1.0f, 0.0f, 0.0f);
    position += movement;

    std::cout << "New Position: ("
        << position.x << ", "
        << position.y << ", "
        << position.z << ")" << std::endl;

    //STB IMAGE TEST
    GLuint texture = loadTexture("assets/textures/example.png");

    glutDisplayFunc(display); // Bind the two functions (above) to respond when necessary
    glutMainLoop(); // To avoid window to be closed

    return 0;
}
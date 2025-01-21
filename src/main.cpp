#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <vector>
#include "Shader.h"

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Function declarations
void display();
void reshape(int width, int height);
void processInput(unsigned char key, int x, int y);
unsigned int loadTexture(const char* path);

struct Fish {
    glm::vec2 position;
    glm::vec2 speed;
    unsigned int textureID;
};

// Globals
std::vector<Fish> fishList;
Shader* shader;
unsigned int VAO, fishTexture;

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("Master Baiting");

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load and compile shaders
    shader = new Shader("../assets/shaders/vertexShader.vs", "../assets/shaders/fragmentShader.fs");

    // Define quad vertices
    float vertices[] = {
        // positions     // texture coords
         0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f,  0.0f, 0.0f,  1.0f, 0.0f,
         0.0f,  0.0f, 0.0f,  0.0f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Load textures
    fishTexture = loadTexture("../assets/textures/fish.png");

    for (int i = 0; i < 5; i++) {
        fishList.push_back({ glm::vec2(-100.0f * i, 100.0f * i), glm::vec2(10.0f, 0.0f), fishTexture });
    }

    // Register GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processInput);
    glutIdleFunc(display);

    glutMainLoop();

    return 0;
}

void display() {
    // Clear screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    // Set up projection
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    shader->setMat4("projection", projection);

    // Render fish
    glBindVertexArray(VAO);

    for (auto& fish : fishList) {
        fish.position += fish.speed * 0.01f;
        if (fish.position.x > SCR_WIDTH) fish.position.x = -100.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(fish.position, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f, 100.0f, 1.0f));
        shader->setMat4("model", model);

        glBindTexture(GL_TEXTURE_2D, fish.textureID);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(unsigned char key, int x, int y) {
    if (key == 27) { // Escape key
        exit(0);
    }
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    std::cout << "Loaded texture: " << path
                  << " (" << width << "x" << height << ", channels: " << nrChannels << ")" << std::endl;

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

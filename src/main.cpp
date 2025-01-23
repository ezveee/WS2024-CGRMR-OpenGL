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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// forward declarations
void initFishData();
float calculateRandomYPosition();
void display();
void reshape(int width, int height);
unsigned int loadTexture(const char* path);
void processInput(unsigned char key, int x, int y);
void mouseCallback(int button, int state, int x, int y);

struct Fish {
    glm::vec2 position;
    glm::vec2 speed;
    unsigned int textureID;
    int pointValue;
    bool isClicked = false;
    float scale = 1.0f;
};

// globals
std::vector<Fish> fishList;
float fishSpeeds[5];
int fishPoints[5];
int score = 0;

Shader* shader;
unsigned int VAO;
unsigned int fishTextures[5];
unsigned int backgroundTexture;


int main(int argc, char** argv) {
    // init GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("Master Baiting");

    // init GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // enable blend for texture transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // init shaders
    shader = new Shader("../assets/shaders/vertexShader.vs", "../assets/shaders/fragmentShader.fs");

    // define quad vertices
    float vertices[] = {
            // positions     // texture coords (x, y, z (0), u, v (coordinates for texture - the same) - used as fish later + NORMALS
            0.0f,  1.0f, 0.0f,     0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // top left
            1.0f,  1.0f, 0.0f,     1.0f, 1.0f,    0.0f, 1.0f, 0.0f, // top right
            1.0f,  0.0f, 0.0f,     1.0f, 0.0f,    0.0f, 1.0f, 0.0f, // bottom right
            0.0f,  0.0f, 0.0f,     0.0f, 0.0f,    0.0f, 1.0f, 0.0f  // bottom left
    }; // basically -> put this vertex of texture on this vertex of quad
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    //Set up to persist vertex data
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); // vertex array object

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    initFishData(); // load fish textures; set fish speeds
    backgroundTexture = loadTexture("../assets/textures/background.png");

    // init fish list
    for (int i = 0; i < 5; i++) {
        fishPoints[i] = (i+1)*10;
        float randomY = calculateRandomYPosition();
        fishList.push_back({
            glm::vec2(-100.0f * i, randomY),
            glm::vec2(fishSpeeds[i], 0.0f),
            fishTextures[i],
            fishPoints[i]
        }); // position, speed, texture, pointValue for each fish
    }

    // register GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processInput);
    glutMouseFunc(mouseCallback);
    glutIdleFunc(display);

    glutMainLoop();

    return 0;
}

void display() {
    // display background texture over entire screen (https://stackoverflow.com/a/31487085)
    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // if not already bound
    glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
                      0, 0, SCR_WIDTH, SCR_HEIGHT,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // apply shader
    shader->use();
    shader->setVec3("lightPos", glm::vec3(SCR_WIDTH/2.0f, SCR_HEIGHT-50.0f, 0.0f));
    shader->setVec3("lightColor", glm::vec3 (1.0f, 1.0f, 1.0f));

    // set up projection
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    shader->setMat4("projection", projection);

    // render each fish
    glBindVertexArray(VAO);

    for (auto& fish : fishList) {
        fish.position += fish.speed * 0.01f;

        // fish offscreen? -> move back to start
        if (fish.position.x > SCR_WIDTH) {
            fish.position.x = -100.0f;
            fish.position.y = calculateRandomYPosition();
        }

        // fish clicked? -> move back to start and increase score
        if (fish.isClicked) {
            fish.scale -= 0.0025f; // fish disappearing animation
            if (fish.scale <= 0.0f) {
                fish.isClicked = false;
                score += fish.pointValue;
                std::cout << "Score: " << score << std::endl;
                fish.scale = 1.0f;
                fish.position.x = -100.0f;
                fish.position.y = calculateRandomYPosition();
            }
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(fish.position, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f * fish.scale, 100.0f * fish.scale, 1.0f));
        shader->setMat4("model", model);

        glBindTexture(GL_TEXTURE_2D, fish.textureID);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    glutSwapBuffers();
}

void initFishData() {
    // load textures
    fishTextures[0] = loadTexture("../assets/textures/fish.png");
    fishTextures[1] = loadTexture("../assets/textures/shrimple.png");
    fishTextures[2] = loadTexture("../assets/textures/cool-fishe.png");
    fishTextures[3] = loadTexture("../assets/textures/shar.png");
    fishTextures[4] = loadTexture("../assets/textures/bluelobster.png");

    // init speeds
    fishSpeeds[0] = 10.0f;
    fishSpeeds[1] = 20.0f;
    fishSpeeds[2] = 30.0f;
    fishSpeeds[3] = 40.0f;
    fishSpeeds[4] = 60.0f;
}

float calculateRandomYPosition() {
    return rand() % (SCR_HEIGHT - 100);
}

// reshape window
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

// keyboard input (only used for esc)
void processInput(unsigned char key, int x, int y) {
    if (key == 27) { // escape key
        exit(0);
    }
}

// mouse input
void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // lmb down
        // convert screen coords to game coords
        float gameX = x;
        float gameY = SCR_HEIGHT - y; // flip Y cause opengls origin is bottom-left

        // check if click intersects any fish
        for (auto& fish : fishList) {
            float fishX = fish.position.x;
            float fishY = fish.position.y;

            // based on fish size
            // did click happen in between start and end of fish texture? -> fish clicked
            if (gameX >= fishX && gameX <= fishX + 100.0f &&
                gameY >= fishY && gameY <= fishY + 100.0f) {
                std::cout << "Fish clicked at (" << fishX << ", " << fishY << ")" << std::endl;
                fish.isClicked = true;
            }
        }
    }
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID); // generate texture name -> stored in textureID

    // texture flipped upside down without this line
    stbi_set_flip_vertically_on_load(true);

    // load image using stb_image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    std::cout << "Loaded texture: " << path
                  << " (ID: " << textureID << " - " << width << "x" << height << ", channels: " << nrChannels << ")" << std::endl;

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB; // either RGBA or RGB based on amount of channels ! doesnt account for grayscale
    glBindTexture(GL_TEXTURE_2D, textureID); // bind texture id to 2D target
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // specify texture from image
    glGenerateMipmap(GL_TEXTURE_2D); // "precomputed, optimized versions of a texture at different levels of detail"
    // "used to improve performance/reduce aliasing artefacts when rendering textures at different distances/scales"

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

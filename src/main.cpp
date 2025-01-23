#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>  // für rand(), srand()
#include <ctime>    // time(NULL) für srand()

#include "Shader.h" // Stelle sicher, dass du Shader.h in deinem Projekt hast.

// --------------------------------------------------------------------------------
// Globale Einstellungen & Forward Declarations
// --------------------------------------------------------------------------------
const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

void initFishData();
void initEnvironment();
glm::vec2 calculateRandomEnvironmentPosition();
glm::vec2 calculateRandomBubblePosition();
int calculateRandomYPosition();
int calculateRandomXPosition();
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

// --------------------------------------------------------------------------------
// Globale Variablen
// --------------------------------------------------------------------------------
std::vector<Fish> fishList;
std::vector<Fish> spawnedFish;

float fishSpeeds[5];
int fishPoints[5];
int score = 0;

Shader* shader       = nullptr;
unsigned int VAO     = 0;

// Fische & Umgebung
unsigned int fishTextures[5];                // Fünf verschiedene Fisch-Texturen
unsigned int backgroundTexture = 0;          // Hintergrund
std::vector<unsigned int> environmentTextures;  // Rock, Seaweed, Coral, Chest
std::vector<glm::vec2> environmentPositions;

// Bubbles
std::vector<unsigned int> bubbleTextures;
std::vector<glm::vec2> bubblePositions;

// Normal Map für Rock
unsigned int rockNormalMap = 0;  // Hier wird rock_normal.jpg geladen

// Kamera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// --------------------------------------------------------------------------------
// main()
// --------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    // Zufallszahlengenerator initialisieren (optional, für Spawn-Positionen)
    srand(static_cast<unsigned>(time(NULL)));

    // 1) GLUT initialisieren
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("OpenGL Game mit Normal Mapping (Rock)");

    // 2) GLEW initialisieren
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // 3) Transparenz aktivieren
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4) Shader laden (angepasste Vertex- und Fragment-Shader mit Normal Mapping)
    //    Bitte Pfade anpassen
    shader = new Shader(
            "../assets/shaders/vertexShader.vs",
            "../assets/shaders/fragmentShader.fs"
    );

    // 5) Ein Quad definieren (Position, TexCoord, Normal)
    float vertices[] = {
            // Positions         // TexCoords   // Normal
            0.0f,  1.0f, 0.0f,   0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
            1.0f,  0.0f, 0.0f,   1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
            0.0f,  0.0f, 0.0f,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    // 6) VAO, VBO, EBO anlegen
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex-Daten
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Index-Daten
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attribute: position(0), texCoord(1), normal(2)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // 7) Umgebung, Fische, etc. initialisieren
    initEnvironment();
    initFishData();
    backgroundTexture = loadTexture("../assets/textures/background.png");

    // Basis-Fische anlegen
    for (int i = 0; i < 5; i++) {
        fishPoints[i] = (i+1)*10;
        fishList.push_back({
                                   glm::vec2(0, 0),             // Position (wird später überschrieben)
                                   glm::vec2(fishSpeeds[i], 0.0f),
                                   fishTextures[i],
                                   fishPoints[i]
                           });
    }

    // Zusätzliche Fische "spawnen" mit zufälligen Positionen
    for(int i = 0; i < 100; i++) {
        int randomIndex = rand() % 5;
        glm::vec2 fishPos;
        fishPos.x = calculateRandomXPosition();
        fishPos.y = calculateRandomYPosition();

        Fish toInsert = {
                fishPos,
                fishList[randomIndex].speed,
                fishList[randomIndex].textureID,
                fishList[randomIndex].pointValue,
                fishList[randomIndex].isClicked,
                fishList[randomIndex].scale
        };
        spawnedFish.push_back(toInsert);
    }

    // 8) GLUT Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processInput);
    glutMouseFunc(mouseCallback);
    glutIdleFunc(display);

    // 9) Hauptschleife
    glutMainLoop();

    return 0;
}

// --------------------------------------------------------------------------------
// Rendering / Display
// --------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Shader aktivieren
    shader->use();

    // 1) Licht-Uniforms (Beispielwerte)
    shader->setVec3("lightPos", glm::vec3(SCR_WIDTH / 2.0f, SCR_HEIGHT * 1.5f, 0.0f));
    shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // 2) Projektionsmatrix (Orthografisch für 2D)
    glm::mat4 projection = glm::ortho(
            0.0f, (float)SCR_WIDTH,
            0.0f, (float)SCR_HEIGHT,
            -1.0f, 1.0f
    );
    shader->setMat4("projection", projection);

    // 3) Viewmatrix (Kamera)
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader->setMat4("view", view);

    // 4) Zeichnen
    glBindVertexArray(VAO);

    // -- 4.1 Hintergrund
    {
        glm::mat4 backgroundModel = glm::mat4(1.0f);
        backgroundModel = glm::translate(
                backgroundModel,
                glm::vec3(
                        -cameraPos.x - (SCR_WIDTH*2),
                        -cameraPos.y - (SCR_HEIGHT*2),
                        0.0f
                )
        );
        backgroundModel = glm::scale(
                backgroundModel,
                glm::vec3(SCR_WIDTH * 6, SCR_HEIGHT * 6, 1.0f)
        );
        shader->setMat4("model", backgroundModel);

        // Nur Diffuse
        glActiveTexture(GL_TEXTURE0);
        shader->setBool("useNormalMap", false);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        // Deine Shader-Uniforms (im Fragment-Shader "texture1")
        shader->setInt("texture1", 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // -- 4.2 Environment (Rock, Seaweed, Coral, Chest)
    for (size_t i = 0; i < environmentPositions.size(); ++i) {
        glm::vec2 position = environmentPositions[i];
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f, 100.0f, 1.0f));
        shader->setMat4("model", model);

        // Wir haben 4 environmentTextures:
        //  - 0 => rock.png
        //  - 1 => seaweed.png
        //  - 2 => coral.png
        //  - 3 => chest.png
        // Rock => i%4 == 0
        int envIndex = i % environmentTextures.size();

        if (envIndex == 0) {
            // = Rock => Diffuse + Normal Map binden
            glActiveTexture(GL_TEXTURE0);
            shader->setBool("useNormalMap", false);
            glBindTexture(GL_TEXTURE_2D, environmentTextures[0]);
            shader->setInt("texture1", 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rockNormalMap);
            shader->setInt("normalMap", 1);
            shader->setBool("useNormalMap", true);

        } else {
            // Seaweed / Coral / Chest => Nur Diffuse
            glActiveTexture(GL_TEXTURE0);
            shader->setBool("useNormalMap", false);
            glBindTexture(GL_TEXTURE_2D, environmentTextures[envIndex]);
            shader->setInt("texture1", 0);
            // Normal Map nicht genutzt -> falls dein Shader "normalMap" abfragt,
            // kann man useNormalMap = 0 setzen (je nach Shader-Code).
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // -- 4.3 Bubbles
    for (size_t i = 0; i < bubblePositions.size(); ++i) {
        glm::vec2 position = bubblePositions[i];
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::scale(model, glm::vec3(150.0f, 150.0f, 1.0f));
        shader->setMat4("model", model);

        glActiveTexture(GL_TEXTURE0);
        shader->setBool("useNormalMap", false);
        glBindTexture(GL_TEXTURE_2D, bubbleTextures[i % bubbleTextures.size()]);
        shader->setInt("texture1", 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // -- 4.4 Fische
    for (auto& fish : spawnedFish) {
        // Bewegung
        fish.position += fish.speed * 0.01f;

        // Respawn
        if (fish.position.x > SCR_WIDTH * 2) {
            fish.position.x = calculateRandomXPosition();
            fish.position.y = calculateRandomYPosition();
        }

        // Klick-Animation (Skalierung)
        if (fish.isClicked) {
            fish.scale -= 0.0025f;
            if (fish.scale <= 0.0f) {
                fish.isClicked = false;
                score += fish.pointValue;
                std::cout << "Score: " << score << std::endl;
                fish.scale = 1.0f;
                fish.position.x = calculateRandomXPosition();
                fish.position.y = calculateRandomYPosition();
            }
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(fish.position, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f * fish.scale, 100.0f * fish.scale, 1.0f));
        shader->setMat4("model", model);

        // Nur Diffuse (Fische haben keine Normal Map)
        glActiveTexture(GL_TEXTURE0);
        shader->setBool("useNormalMap", false);
        glBindTexture(GL_TEXTURE_2D, fish.textureID);
        shader->setInt("texture1", 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    glutSwapBuffers();
}

// --------------------------------------------------------------------------------
// Umgebung + Bubbles
// --------------------------------------------------------------------------------
void initEnvironment()
{
    // 1) Environment-Texturen laden
    environmentTextures.push_back(loadTexture("../assets/textures/rock.png"));      // Index 0: Rock
    environmentTextures.push_back(loadTexture("../assets/textures/seaweed.png"));   // Index 1
    environmentTextures.push_back(loadTexture("../assets/textures/coral.png"));     // Index 2
    environmentTextures.push_back(loadTexture("../assets/textures/chest.png"));     // Index 3

    // 2) Normal Map für Rock
    rockNormalMap = loadTexture("../assets/textures/rock-normal.jpg");  // Pfad ggf. anpassen

    // 3) Zufällige Positionen für Environment
    for (int i = 0; i < 28; ++i) {
        environmentPositions.push_back(calculateRandomEnvironmentPosition());
    }

    // 4) Bubbles laden
    bubbleTextures.push_back(loadTexture("../assets/textures/bubble1.png"));
    bubbleTextures.push_back(loadTexture("../assets/textures/bubble2.png"));
    bubbleTextures.push_back(loadTexture("../assets/textures/bubble3.png"));

    // 5) Zufällige Positionen für Bubbles
    for (int i = 0; i < 21; ++i) {
        bubblePositions.push_back(calculateRandomBubblePosition());
    }
}

glm::vec2 calculateRandomEnvironmentPosition()
{
    glm::vec2 randomPos;
    randomPos.x = -800 + (rand() % 2381);
    randomPos.y = -585 - (rand() % 16);
    return randomPos;
}

glm::vec2 calculateRandomBubblePosition()
{
    glm::vec2 randomPos;
    randomPos.x = -800 + (rand() % 2401);
    randomPos.y = -600 + (rand() % 1801);
    return randomPos;
}

// --------------------------------------------------------------------------------
// Fische
// --------------------------------------------------------------------------------
void initFishData()
{
    // Texturen
    fishTextures[0] = loadTexture("../assets/textures/fish.png");
    fishTextures[1] = loadTexture("../assets/textures/shrimple.png");
    fishTextures[2] = loadTexture("../assets/textures/cool-fishe.png");
    fishTextures[3] = loadTexture("../assets/textures/shar.png");
    fishTextures[4] = loadTexture("../assets/textures/bluelobster.png");

    // Geschwindigkeiten
    fishSpeeds[0] = 10.0f;
    fishSpeeds[1] = 20.0f;
    fishSpeeds[2] = 30.0f;
    fishSpeeds[3] = 40.0f;
    fishSpeeds[4] = 60.0f;
}

int calculateRandomYPosition()
{
    int randomSign = rand() % 2;
    int result = rand() % (SCR_HEIGHT*2);
    if (randomSign == 0) {
        return -result;
    }
    return result;
}

int calculateRandomXPosition()
{
    int randomSign = rand() % 2;
    int result = rand() % (SCR_WIDTH*2);
    if (randomSign == 0) {
        return -result;
    }
    return result;
}

// --------------------------------------------------------------------------------
// Reshape / Input
// --------------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
}

float cameraSpeed = 4.0f;

// Tastatur
void processInput(unsigned char key, int x, int y)
{
    if (key == 27) { // ESC
        exit(0);
    }

    glm::vec2 direction(0.0f);

    if (key == 'w') {
        if(cameraPos.y + 1.0f < SCR_HEIGHT){
            direction.y += 1.0f;
        }
    }
    if (key == 's') {
        if(cameraPos.y - 1.0f > -(int)SCR_HEIGHT){
            direction.y -= 1.0f;
        }
    }
    if (key == 'a') {
        if(cameraPos.x - 1.0f > -(int)SCR_WIDTH){
            direction.x -= 1.0f;
        }
    }
    if (key == 'd') {
        if(cameraPos.x + 1.0f < SCR_WIDTH){
            direction.x += 1.0f;
        }
    }

    if (direction != glm::vec2(0.0f)) {
        direction = glm::normalize(direction);
    }
    cameraPos += glm::vec3(direction.x, direction.y, 0.0f) * cameraSpeed;
}

// Maus
void mouseCallback(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Klick in "Spielkoordinaten" umrechnen
        float gameX = x + cameraPos.x;
        float gameY = SCR_HEIGHT - y + cameraPos.y;

        // Prüfen, ob ein Fisch geklickt wurde
        for (auto& fish : spawnedFish) {
            float fishX = fish.position.x;
            float fishY = fish.position.y;

            if (gameX >= fishX && gameX <= fishX + 100.0f &&
                gameY >= fishY && gameY <= fishY + 100.0f) {
                std::cout << "Fish clicked at ("
                          << fishX << ", " << fishY << ")" << std::endl;
                fish.isClicked = true;
            }
        }
    }
}

// --------------------------------------------------------------------------------
// Textur laden (stb_image)
// --------------------------------------------------------------------------------
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // stb_image: Bild beim Laden flippen
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    std::cout << "Loading texture: " << path << " => ID: " << textureID
              << " (" << width << "x" << height
              << ", channels = " << nrChannels << ")" << std::endl;

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    // RGBA oder RGB?
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Daten in die Texture laden
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(
            GL_TEXTURE_2D, 0, format,
            width, height, 0,
            format, GL_UNSIGNED_BYTE, data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture-Filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

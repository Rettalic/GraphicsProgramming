#include <iostream>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "utils.h"

struct MyFrameBuffer
{
    unsigned int ID = 0;
    unsigned int color1 = 0, color2 = 0, depth = 0;
    MyFrameBuffer() {}      
};
 
MyFrameBuffer screenBuffer;

// forward declaration
void renderTerrain(glm::mat4 view, glm::mat4 projection);
void renderSkybox(glm::mat4 view, glm::mat4 projection);
void setupResources();
void renderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection);
void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID);
void renderToBuffer(MyFrameBuffer to, MyFrameBuffer from, unsigned int shader);
void renderQuad();

glm::vec3 cameraPosition(100, 100, 100), cameraForward(0, 0, 1), cameraUp(0, 1, 0);
glm::vec3 lightDirection(-0.5, -0.25, -0.5);

unsigned int plane, planeSize, VAO, cubeSize;
unsigned int terrainProgram, skyProgram, modelProgram, blitProgram, chrabbProgram;

// textures
unsigned int heightmapID;
unsigned int heightNormalID;
unsigned int dirtID, sandID, grassID, rockID, snowID;
MyFrameBuffer post1, post2, scene;

// models
Model* backpack;
Model* blockje;

void handleInput(GLFWwindow* window, float deltaTime) {
    static bool w, s, a, d, space, ctrl;
    static double cursorX = -1, cursorY = -1, lastCursorX, lastCursorY;
    static float speed = 100.0f;
    static float pitch = 0, yaw = 0;
    float sensitivity = 100.0f * deltaTime;
    float step = speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)        w = true;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) w = false;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)        s = true;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) s = false;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)        a = true;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) a = false;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)        d = true;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) d = false;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)        space = true;
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) space = false;

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)        ctrl = true;
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) ctrl = false;

    if (cursorX == -1) {
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }

    lastCursorX = cursorX;
    lastCursorY = cursorY;

    glfwGetCursorPos(window, &cursorX, &cursorY);
    glm::vec2 mouseDelta(cursorX - lastCursorX, cursorY - lastCursorY);

    // calc move
    yaw -= mouseDelta.x * sensitivity;
    pitch += mouseDelta.y * sensitivity;
    if (pitch < -90.0f) pitch = -90.0f;
    else if (pitch > 90.0f) pitch = 90.0f;

    if (yaw < -180.0f) yaw += 360;
    else if (yaw > 180.0f) yaw -= 360;

    glm::vec3 euler(glm::radians(pitch), glm::radians(yaw), 0);
    glm::quat q(euler);

    // update cam
    glm::vec3 translation(0, 0, 0);

    // movement
    if (w) translation.z += speed * deltaTime;
    if (s) translation.z -= speed * deltaTime;
    if (a) translation.x += speed * deltaTime;
    if (d) translation.x -= speed * deltaTime;
    if (space) translation.y += speed * deltaTime;
    if (ctrl) translation.y -= speed * deltaTime;
    cameraPosition += q * translation;
    cameraUp = q * glm::vec3(0, 1, 0);
    cameraForward = q * glm::vec3(0, 0, 1);
}

int main()
{
    static double previousT = 0;
    std::cout << "Hello World! Time to start!\n";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    int width  = 1920;
    int height = 1080;

    GLFWwindow* window = glfwCreateWindow(width, height, "Pc heater enabler", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);
    setupResources();

    createFrameBuffer(width, height, post1.ID, post1.color1, post1.depth);
    createFrameBuffer(width, height, post2.ID, post2.color1, post2.depth);
    createFrameBuffer(width, height, scene.ID, scene.color1, scene.depth);

    glm::mat4 projection = glm::perspective(glm::radians(65.0f), width / (float)height, 0.05f, 500.0f);
    glm::mat4 view;

    // OPENGL SETTINGS 
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    int heightmapWidth, heightmapheight;
    int channels = 4;
    unsigned char* data;

    data = stbi_load("Heightmap.png", &heightmapWidth, &heightmapheight, &channels, 4);
    if (data == nullptr) {
        std::cout << "ERROR" << std::endl;
    }

    glm::vec3 positionModels[10];
    for (int i = 0; i < 10; i++)
    {
        positionModels[i] = glm::vec3(rand() % 500, 0, rand() % 500);
        positionModels[i].y = data[(int)(positionModels[i].z * heightmapWidth + positionModels[i].x)* 4]/255.0f;
        positionModels[i].y *= 100.0f;
    }

    while (!glfwWindowShouldClose(window)) {
        double t = glfwGetTime();
        float deltaTime = t - previousT;
        previousT = t;
        handleInput(window, deltaTime);

        view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);

        glBindFramebuffer(GL_FRAMEBUFFER, scene.ID);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSkybox(view, projection);
        renderTerrain(view, projection);

        for (int i = 0; i < 5; i++)
        {
         // renderModel(backpack, modelProgram, positionModels[i], glm::vec3(0, 0, 0), 4.0f, view, projection);
          renderModel(blockje,  modelProgram, positionModels[i], glm::vec3(0, 0, 0), 4.0f, view, projection);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Post FX
        renderToBuffer(post1, scene, chrabbProgram);


        // Blit to screen
        renderToBuffer(screenBuffer, post1, blitProgram);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void renderTerrain(glm::mat4 view, glm::mat4 projection) {
    // TERRAIN
    glUseProgram(terrainProgram);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, glm::vec3(0, 0, 0));

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE,
        glm::value_ptr(world));

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1,
        GL_FALSE, glm::value_ptr(projection));

    glUniform3f(glGetUniformLocation(terrainProgram, "cameraPosition"),
        cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glUniform3f(glGetUniformLocation(terrainProgram, "lightDirection"),
        lightDirection.x, lightDirection.y, lightDirection.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightNormalID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirtID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sandID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grassID);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rockID);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, snowID);
    glBindVertexArray(plane);
    glDrawElements(GL_TRIANGLES, planeSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderSkybox(glm::mat4 view, glm::mat4 projection) {
    // SKY BOX
    glUseProgram(skyProgram);
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);
    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, cameraPosition);
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"),      1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(skyProgram, "cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(glGetUniformLocation(skyProgram, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, cubeSize, GL_UNSIGNED_INT, 0);
}

void setupResources() {
    //backpack = new Model("backpack/backpack.obj");
    blockje  = new Model("blockje/untitled.obj");

    stbi_set_flip_vertically_on_load(true);
    // need 24 vertices for normal/uv-mapped Cube
    float vertices[] = {
        // positions            //colors            // tex coords   // normals
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 0.0f,   1.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   -1.f, 1.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   -1.f, 0.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, -1.f,      0.f, 0.f, -1.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, -1.f,      0.f, 0.f, -1.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   3.f, 0.f,       0.f, 1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   3.f, 1.f,       0.f, 1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       0.f, 1.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       0.f, 1.f, 0.f
    };
    unsigned int indices[] = {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    cubeSize = sizeof(indices);
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    int stride = sizeof(float) * 11;

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);
    // normal

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 8));
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    ///END SETUP CUBE///

    plane = GeneratePlane("Heightmap.png", GL_RGBA, 4, 1.0f, 2.0f, planeSize, heightmapID);

    // terrain textures
    heightNormalID = loadTexture("heightnormal.png", GL_RGBA, 4);
    dirtID  = loadTexture("dirt.jpg",   GL_RGB,  3);
    sandID  = loadTexture("sand.jpg",   GL_RGB,  3);
    grassID = loadTexture("grass.png",  GL_RGBA, 4);
    rockID  = loadTexture("rock.jpg",   GL_RGB,  3);
    snowID  = loadTexture("snow.jpg",   GL_RGB,  3);

    ///SETUP SHADER PROGRAMS///
    unsigned int vertSky, fragSky, vertModel, fragModel;
    unsigned int vertID, fragID, vertImg, fragImg, fragChrabb;

    CreateShader("vertexShaderSky.shader",   GL_VERTEX_SHADER,   vertSky);
    CreateShader("vertexShader.shader",      GL_VERTEX_SHADER,   vertID);
    CreateShader("vertimg.shader",           GL_VERTEX_SHADER,   vertImg);
    CreateShader("vertexShaderSky.shader",   GL_VERTEX_SHADER,   vertSky);
    CreateShader("vertModel.shader",         GL_VERTEX_SHADER,   vertModel);

    CreateShader("fragmentShaderSky.shader", GL_FRAGMENT_SHADER, fragSky);
    CreateShader("fragmentShader.shader",    GL_FRAGMENT_SHADER, fragID);
    CreateShader("fragimg.shader",           GL_FRAGMENT_SHADER, fragImg);
    CreateShader("fragmentShaderSky.shader", GL_FRAGMENT_SHADER, fragSky);
    CreateShader("fragModel.shader",         GL_FRAGMENT_SHADER, fragModel);
    CreateShader("frag_chrabb.shader",       GL_FRAGMENT_SHADER, fragChrabb);

    terrainProgram = glCreateProgram();
    glAttachShader(terrainProgram, vertID);
    glAttachShader(terrainProgram, fragID);
    glLinkProgram(terrainProgram);

    skyProgram = glCreateProgram();
    glAttachShader(skyProgram, vertSky);
    glAttachShader(skyProgram, fragSky);
    glLinkProgram(skyProgram);

    blitProgram = glCreateProgram();
    glAttachShader(blitProgram, vertImg);
    glAttachShader(blitProgram, fragImg);
    glLinkProgram(blitProgram);

    chrabbProgram = glCreateProgram();
    glAttachShader(chrabbProgram, vertImg);
    glAttachShader(chrabbProgram, fragChrabb);
    glLinkProgram(chrabbProgram);

    modelProgram = glCreateProgram();
    glAttachShader(modelProgram, vertModel);
    glAttachShader(modelProgram, fragModel);
    glLinkProgram(modelProgram);

    glDeleteShader(vertID);
    glDeleteShader(fragID);
    glDeleteShader(vertSky);
    glDeleteShader(fragSky);
    glDeleteShader(vertImg);
    glDeleteShader(fragChrabb);

    ///END SETUP SHADER PROGRAM///
    glUseProgram(terrainProgram);
    glUniform1i(glGetUniformLocation(terrainProgram, "heightmap"), 0);
    glUniform1i(glGetUniformLocation(terrainProgram, "normalmap"), 1);
    glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
    glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
    glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
    glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
    glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);
}

void renderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection) {
    // shader gebruiker
    glUseProgram(shader);
    glEnable(GL_DEPTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // world matrix bouwen
    glm::mat4 world = glm::mat4(1);
    world = glm::translate(world, position);
    world = glm::scale(world, glm::vec3(scale));
    glm::quat q(rotation);
    world = world * glm::toMat4(q);

    // shader instellingen
    glUniformMatrix4fv(glGetUniformLocation(shader, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
    glUniform3f(glGetUniformLocation(shader, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);

    model->Draw(shader);
}

void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID) {

    // generate frame buffer
    glGenFramebuffers(1, &frameBufferID);

    // generate color buffer
    glGenTextures(1, &colorBufferID);
    glBindTexture(GL_TEXTURE_2D, colorBufferID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    // generate depth buffer
    glGenRenderbuffers(1, &depthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderToBuffer(MyFrameBuffer to, MyFrameBuffer from, unsigned int shader) {
    glBindFramebuffer(GL_FRAMEBUFFER, to.ID);
    glUseProgram(shader);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, from.color1);
    
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// renderQuad() renders a 1x1 XY quad in NDC
unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
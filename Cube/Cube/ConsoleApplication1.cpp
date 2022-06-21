#include <iostream>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
void loadFromFile(const char* url, char** target) {
    std::ifstream stream(url, std::ios::binary);
    stream.seekg(0, stream.end);
    int total = stream.tellg();
    *target = new char[total + 1];
    stream.seekg(0, stream.beg);
    stream.read(*target, total);
    (*target)[total] = '\0';
    stream.close();
}
unsigned int loadTexture(std::string url, GLenum format) {
    // gen & bind IDs
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // instellen texture params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // shit inladen
    int width, height, channels;
    unsigned char* data;
    data = stbi_load(url.c_str(), &width, &height, &channels, 0);
    if (data == nullptr) {
        std::cout << "Error loading file: " << url.c_str() << std::endl;
    }
    else {
        // shit aan opengl geven
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}
int main()
{
    std::cout << "Hello World!\n";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 800;
    int height = 600;
    GLFWwindow* window = glfwCreateWindow(width, height, "Hello OpenGL!", nullptr,
        nullptr);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, width, height);
    ///SETUP OBJECT///
    // Vertices of our triangle!
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
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
        GL_STATIC_DRAW);
    int stride = sizeof(float) * 11;
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) *
        3));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) *
        6));
    glEnableVertexAttribArray(2);
    // normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) *
        8));
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    ///END SETUP OBJECT///
    ///SETUP SHADER PROGRAM///
    char* vertexSource;
    loadFromFile("vertexShader.shader", &vertexSource);
    char* fragmentSource;
    loadFromFile("fragmentShader.shader", &fragmentSource);
    // LOAD & CREATE TEXTURES

    //unsigned int diffuseTexID = loadTexture("container2.png", GL_RGBA);
    unsigned int diffuseTexID = loadTexture("randomImage.jpg", GL_RGB);
    // END
    unsigned int vertID, fragID;
    vertID = glCreateShader(GL_VERTEX_SHADER);
    fragID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertID, 1, &vertexSource, nullptr);
    glShaderSource(fragID, 1, &fragmentSource, nullptr);
    int success;
    char infoLog[512];
    glCompileShader(vertID);
    glGetShaderiv(vertID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog <<
            std::endl;
    };
    glCompileShader(fragID);
    glGetShaderiv(fragID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog <<
            std::endl;
    };
    unsigned int myProgram = glCreateProgram();
    glAttachShader(myProgram, vertID);
    glAttachShader(myProgram, fragID);
    glLinkProgram(myProgram);
    glDeleteShader(vertID);
    glDeleteShader(fragID);
    ///END SETUP SHADER PROGRAM///
    /// MATRIX SETUP ///
    glUseProgram(myProgram);
    int worldLoc = glGetUniformLocation(myProgram, "world");
    int viewLoc = glGetUniformLocation(myProgram, "view");
    int projLoc = glGetUniformLocation(myProgram, "projection");
    /// END MATRIX SETUP ///
    // OPENGL SETTINGS //
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    while (!glfwWindowShouldClose(window)) {
        double t = glfwGetTime();
        float r = (float)sin(t * 1.1f);
        float g = (float)sin(t * 1.3f);
        float b = (float)sin(t * 1.7f);
        glm::mat4 world = glm::mat4(1.f);
        world = glm::rotate(world, glm::radians((float)t * 45.0f), glm::vec3(0, 0,
            1));
        world = glm::scale(world, glm::vec3(1, 1, 1));
        world = glm::translate(world, glm::vec3(0, 0, 0));
        glm::mat4 view = glm::lookAt(glm::vec3(0, 3, -3), glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(65.0f), width /
            (float)height, 0.1f, 100.0f);
        glUniformMatrix4fv(worldLoc, 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        // iets tekenen
        //glClearColor(r,g,b,1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexID);
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)
            (3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH = 1280;
const int HEIGHT = 720;

int mode = 1;
int primIndex = 0;
bool wireframe = false;

GLenum primitives[] = { GL_TRIANGLES, GL_LINE_LOOP, GL_LINE_STRIP, GL_POINTS };
const char* primNames[] = { "GL_TRIANGLES", "GL_LINE_LOOP", "GL_LINE_STRIP", "GL_POINTS" };

bool prevKeys[GLFW_KEY_LAST + 1] = { false };

bool pressedOnce(GLFWwindow* window, int key) {
    bool now = glfwGetKey(window, key) == GLFW_PRESS;
    bool fired = now && !prevKeys[key];
    prevKeys[key] = now;
    return fired;
}

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float deltaTime, float& speed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (pressedOnce(window, GLFW_KEY_1)) { mode = 1; std::cout << "Rezhim 1: Bazalyq qozhalys\n"; }
    if (pressedOnce(window, GLFW_KEY_2)) { mode = 2; std::cout << "Rezhim 2: Ortalyqta pulsatsiya\n"; }
    if (pressedOnce(window, GLFW_KEY_3)) { mode = 3; std::cout << "Rezhim 3: Eki uşbūryş\n"; }
    if (pressedOnce(window, GLFW_KEY_4)) { mode = 4; std::cout << "Rezhim 4: Tort uşbūryş\n"; }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        speed += 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        speed -= 2.0f * deltaTime;

    if (pressedOnce(window, GLFW_KEY_P)) {
        primIndex = (primIndex + 1) % 4;
        std::cout << "Primitiv: " << primNames[primIndex] << "\n";
    }

    if (pressedOnce(window, GLFW_KEY_TAB)) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
}

unsigned int makeShader(GLenum type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform vec2 uOffset;
uniform float uScale;

out vec3 ourColor;

void main() {
    vec2 pos = aPos.xy * uScale + uOffset;
    gl_Position = vec4(pos, aPos.z, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW iske qosylmady!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Seminar 05 - Interactive Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Tereze zhasalmady!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    // GLEW қолданбаймыз, сондықтан glewInit() шақырылмайды (қате жоқ!)
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Basqaru: 1-4 rezhim, W/S - zhyldamdyq, P - primitiv, TAB - wireframe, ESC - shygu\n";

    unsigned int vertexShader = makeShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSrc);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
    int uScaleLoc = glGetUniformLocation(shaderProgram, "uScale");

    float vertices[] = {
         0.0f,  0.2f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.2f, -0.2f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.2f, -0.2f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glPointSize(10.0f);

    float lastFrame = (float)glfwGetTime();
    float angle = 0.0f;
    float speed = 2.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window, deltaTime, speed);

        angle += speed * deltaTime;
        float radius = 0.5f;
        float offsetX = std::cos(angle) * radius;
        float offsetY = std::sin(angle) * radius;
        float scale = 1.0f + 0.3f * std::sin(currentFrame * 3.0f);

        GLenum prim = primitives[primIndex];

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        if (mode == 1) {
            glUniform2f(uOffsetLoc, offsetX, offsetY);
            glUniform1f(uScaleLoc, scale);
            glDrawArrays(prim, 0, 3);
        }
        else if (mode == 2) {
            glUniform2f(uOffsetLoc, 0.0f, 0.0f);
            glUniform1f(uScaleLoc, scale * 1.5f);
            glDrawArrays(prim, 0, 3);
        }
        else if (mode == 3) {
            glUniform2f(uOffsetLoc, offsetX, offsetY);
            glUniform1f(uScaleLoc, scale);
            glDrawArrays(prim, 0, 3);

            glUniform2f(uOffsetLoc, -offsetX, -offsetY);
            glUniform1f(uScaleLoc, scale);
            glDrawArrays(prim, 0, 3);
        }
        else if (mode == 4) {
            glUniform2f(uOffsetLoc, -0.5f,  0.5f); glUniform1f(uScaleLoc, 0.8f); glDrawArrays(prim, 0, 3);
            glUniform2f(uOffsetLoc,  0.5f,  0.5f); glUniform1f(uScaleLoc, 0.8f); glDrawArrays(prim, 0, 3);
            glUniform2f(uOffsetLoc, -0.5f, -0.5f); glUniform1f(uScaleLoc, 0.8f); glDrawArrays(prim, 0, 3);
            glUniform2f(uOffsetLoc,  0.5f, -0.5f); glUniform1f(uScaleLoc, 0.8f); glDrawArrays(prim, 0, 3);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
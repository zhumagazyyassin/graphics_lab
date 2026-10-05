#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

const int WIDTH = 1280;
const int HEIGHT = 720;

// =====================================================
// VERTEX SHADER
// =====================================================

const char* vertexSrc = R"(
#version 330 core

layout (location = 0) in vec3 aPos;   // Позиция атрибуты (location = 0)
layout (location = 1) in vec3 aColor; // Түс атрибуты (location = 1)

out vec3 ourColor; // Фрагменттік шейдерге түсті жіберу

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor; // Түсті келесі кезеңге тасымалдау
}
)";

// =====================================================
// FRAGMENT SHADER
// =====================================================

const char* fragmentSrc = R"(
#version 330 core

in vec3 ourColor; // Вершиналық шейдерден келген түс
out vec4 FragColor;

void main()
{
    // Негізгі түрлі-түсті шығару:
    FragColor = vec4(ourColor, 1.0);
}
)";

// =====================================================
// HELPER: makeShader() Функциясы
// =====================================================

unsigned int makeShader(GLenum type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

// =====================================================
// WINDOW RESIZE CALLBACK
// =====================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// =====================================================
// MAIN
// =====================================================

int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW іске қосылмады!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Seminar 03 - Multi-color Triangle", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Терезе жасалмады!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // WSL2 ӨЧӘН ТӘЗӘТЕЛГӘН GLEW ИНИЦИАЛИЗАЦИЯСЫ
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    
    // WSL2-дә GLEW еш кына GL_INVALID_ENUM (1280) яки башка хата кайтара,
    // кайтарылган хатаны үткәреп җибәрәбез һәм OpenGL функцияләренең эшләвен тикшерәбез:
    if (glewInit != nullptr) 
    {
        glGetError(); // Тәрәзә хатасын тазарттыҡ
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";

    // =================================================
    // SHADERS & PROGRAM
    // =================================================

    unsigned int vertexShader = makeShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSrc);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // =================================================
    // VERTEX DATA
    // =================================================

    float vertices[] = {
        // Позициялар (X, Y, Z)     // Түстер (R, G, B)
         0.0f,  0.5f, 0.0f,        1.0f, 0.0f, 0.0f,  // Жоғарғы - Қызыл
        -0.5f, -0.5f, 0.0f,        0.0f, 1.0f, 0.0f,  // Сол - Жасыл
         0.5f, -0.5f, 0.0f,        0.0f, 0.0f, 1.0f   // Оң - Көк
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 1. Позиция атрибуты (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Түс атрибуты (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // =================================================
    // RENDER LOOP
    // =================================================

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
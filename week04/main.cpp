#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH = 1280;
const int HEIGHT = 720;

// Переменные для управления скоростью и временем (dt)
float speed = 2.0f;     // Угловая скорость движения (радиан/сек)
float angle = 0.0f;     // Текущий угол
float lastFrame = 0.0f; // Время предыдущего кадра

// Обработка ввода (управление скоростью клавишами W / S)
void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W — увеличить скорость
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        speed += 2.0f * deltaTime;

    // S — уменьшить скорость
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        speed -= 2.0f * deltaTime;
}

// =====================================================
// VERTEX SHADER
// =====================================================
const char* vertexSrc = R"(
#version 330 core

layout (location = 0) in vec3 aPos;   // Позиция
layout (location = 1) in vec3 aColor; // Цвет

out vec3 ourColor;

uniform vec2 uOffset; // Смещение по окружности (X, Y)
uniform float uScale; // Масштаб для пульсации

void main()
{
    // Масштабируем вершины треугольника и добавляем смещение центра
    vec2 pos = aPos.xy * uScale + uOffset;
    gl_Position = vec4(pos, aPos.z, 1.0);
    ourColor = aColor;
}
)";

// =====================================================
// FRAGMENT SHADER
// =====================================================
const char* fragmentSrc = R"(
#version 330 core

in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

// Helper: компиляция шейдера
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW не инициализирован!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Seminar 04 - Moving Triangle", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Не удалось создать окно!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // Очистка возможной ошибки инициализации GLEW в WSL2

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

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Кэширование локаций uniform-переменных
    int uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
    int uScaleLoc = glGetUniformLocation(shaderProgram, "uScale");

    // =================================================
    // VERTEX DATA (Небольшой треугольник)
    // =================================================
    float vertices[] = {
        // Position          // Color
         0.0f,  0.2f, 0.0f,  1.0f, 0.0f, 0.0f, // Верх (Красный)
        -0.2f, -0.2f, 0.0f,  0.0f, 1.0f, 0.0f, // Лево (Зеленый)
         0.2f, -0.2f, 0.0f,  0.0f, 0.0f, 1.0f  // Право (Синий)
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // =================================================
    // RENDER LOOP
    // =================================================
    while (!glfwWindowShouldClose(window))
    {
        // 1. Расчёт delta time (dt)
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 2. Обработка ввода (W/S)
        glfwPollEvents();
        processInput(window, deltaTime);

        // 3. Обновление логики (движение по окружности и пульсация)
        angle += speed * deltaTime;
        float radius = 0.5f;
        float offsetX = std::cos(angle) * radius;
        float offsetY = std::sin(angle) * radius;

        // Пульсация (масштаб от 0.7 до 1.3)
        float scale = 1.0f + 0.3f * std::sin(currentFrame * 3.0f);

        // 4. Отрисовка
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Передача uniform-значений
        glUniform2f(uOffsetLoc, offsetX, offsetY);
        glUniform1f(uScaleLoc, scale);

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
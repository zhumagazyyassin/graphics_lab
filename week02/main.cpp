#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

const int WIDTH = 1280;
const int HEIGHT = 720;

bool spacePressed = false;

// =====================================================
// VERTEX SHADER
// =====================================================

const char* vertexSrc = R"(
#version 330 core

layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// =====================================================
// FRAGMENT SHADER
// =====================================================

const char* fragmentSrc = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    // Қызғылт-сары түс
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

// =====================================================
// WINDOW RESIZE CALLBACK
// =====================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// =====================================================
// KEYBOARD CALLBACK
// =====================================================

void key_callback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods)
{
    // ESC — шығу
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // SPACE
    if (key == GLFW_KEY_SPACE)
    {
        if (action == GLFW_PRESS)
        {
            spacePressed = true;
            std::cout << "SPACE: PRESSED\n";
        }

        if (action == GLFW_RELEASE)
        {
            spacePressed = false;
            std::cout << "SPACE: RELEASED\n";
        }
    }
}

// =====================================================
// MAIN
// =====================================================

int main()
{
    // =================================================
    // GLFW
    // =================================================

    if (!glfwInit())
    {
        std::cerr << "GLFW іске қосылмады!\n";
        return -1;
    }

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Computer Graphics - Week 02",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Терезе жасалмады!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback
    );

    glfwSetKeyCallback(
        window,
        key_callback
    );

    // VSync өшіру
    glfwSwapInterval(0);

    // =================================================
    // GLAD
    // =================================================

    if (gladLoadGL(glfwGetProcAddress) == 0)
    {
        std::cerr << "GLAD жүктелмеді!\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: "
              << glGetString(GL_VERSION)
              << "\n";

    std::cout << "GPU: "
              << glGetString(GL_RENDERER)
              << "\n";

    // =================================================
    // SHADERS
    // =================================================

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexSrc,
        nullptr
    );

    glCompileShader(vertexShader);

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentSrc,
        nullptr
    );

    glCompileShader(fragmentShader);

    // Shader Program
    unsigned int shader =
        glCreateProgram();

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);

    glLinkProgram(shader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // =================================================
    // GRID
    // =================================================

    std::vector<float> vertices;

    const int ROWS = 8;
    const int COLS = 12;

    float cellWidth = 2.0f / COLS;
    float cellHeight = 2.0f / ROWS;

    // Әр квадрат үшін жоғарғы үшбұрыш
    //
    // A -------- B
    // |        / |
    // |      /   |
    // |    /     |
    // C -------- D
    //
    // Диагональ: A -> D
    // Боялатын үшбұрыш: A-B-D

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            float left =
                -1.0f + col * cellWidth;

            float right =
                left + cellWidth;

            float top =
                1.0f - row * cellHeight;

            float bottom =
                top - cellHeight;

            // Үшбұрыш A-B-D

            vertices.push_back(left);
            vertices.push_back(top);

            vertices.push_back(right);
            vertices.push_back(top);

            vertices.push_back(right);
            vertices.push_back(bottom);
        }
    }

    // =================================================
    // VAO + VBO
    // =================================================

    unsigned int vao;
    unsigned int vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // X, Y координаттары
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // =================================================
    // FPS
    // =================================================

    double lastTime = glfwGetTime();
    int frameCount = 0;

    // =================================================
    // MAIN LOOP
    // =================================================

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float time =
            static_cast<float>(glfwGetTime());

        // Анимациялық фон
        float red =
            (std::sin(time * 2.0f) + 1.0f) / 2.0f;

        float green =
            (std::sin(time * 3.0f) + 1.0f) / 2.0f;

        float blue =
            (std::sin(time * 4.0f) + 1.0f) / 2.0f;

        // SPACE басылып тұрса — ақ фон
        if (spacePressed)
        {
            glClearColor(
                1.0f,
                1.0f,
                1.0f,
                1.0f
            );
        }
        else
        {
            glClearColor(
                red,
                green,
                blue,
                1.0f
            );
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // =================================================
        // DRAW GRID
        // =================================================

        glUseProgram(shader);

        glBindVertexArray(vao);

        glDrawArrays(
            GL_TRIANGLES,
            0,
            static_cast<GLsizei>(vertices.size() / 2)
        );

        glBindVertexArray(0);

        glfwSwapBuffers(window);

        // =================================================
        // FPS
        // =================================================

        frameCount++;

        double currentTime = glfwGetTime();

        if (currentTime - lastTime >= 1.0)
        {
            std::cout << "FPS: "
                      << frameCount
                      << "\n";

            frameCount = 0;
            lastTime = currentTime;
        }
    }

    // =================================================
    // CLEANUP
    // =================================================

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwTerminate();

    return 0;
}

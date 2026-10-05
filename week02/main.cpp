#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

const int WIDTH = 1280;
const int HEIGHT = 720;

// SPACE басылып тұр ма?
bool spacePressed = false;

// =====================================================
// SHADERS
// =====================================================

// Vertex Shader
const char* vertexSrc = R"(
#version 330 core

layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader
const char* fragmentSrc = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    // Жоғарғы үшбұрыштың түсі
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

// =====================================================
// Терезе өлшемі өзгерген кезде
// =====================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// =====================================================
// Пернетақта callback
// =====================================================

void key_callback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods)
{
    // ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // SPACE
    if (key == GLFW_KEY_SPACE)
    {
        // SPACE басылды
        if (action == GLFW_PRESS)
        {
            spacePressed = true;
            std::cout << "SPACE: PRESSED\n";
        }

        // SPACE жіберілді
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
    // GLFW іске қосу
    // =================================================

    if (!glfwInit())
    {
        std::cerr << "GLFW іске қосылмады!\n";
        return -1;
    }

    // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // 1280x720 терезе
    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Computer Graphics - Grid",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Терезе жасалмады!\n";
        glfwTerminate();
        return -1;
    }

    // OpenGL context
    glfwMakeContextCurrent(window);

    // Callback-тар
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
    // SHADER COMPILE
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
    // ГРАФИКАЛЫҚ ТОР
    // =================================================

    std::vector<float> vertices;

    // Тор өлшемі
    const int ROWS = 8;
    const int COLS = 12;

    // Бір квадраттың өлшемі
    float cellWidth = 2.0f / COLS;
    float cellHeight = 2.0f / ROWS;

    // =================================================
    // ӘР КВАДРАТТЫ 2 ҮШБҰРЫШҚА БӨЛУ
    // ЖОҒАРҒЫ ҮШБҰРЫШТЫ ҚЫЗҒЫЛТ САРЫ ЕТУ
    // =================================================

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            // Квадраттың координаттары

            float left =
                -1.0f + col * cellWidth;

            float right =
                left + cellWidth;

            float top =
                1.0f - row * cellHeight;

            float bottom =
                top - cellHeight;

            // =================================================
            // ЖОҒАРҒЫ ҮШБҰРЫШ
            // Диагональ: жоғарғы сол жақ -> төменгі оң жақ
            // =================================================

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

    // Әр vertex-та 2 координата: X, Y
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
    // НЕГІЗГІ ЦИКЛ
    // =================================================

    while (!glfwWindowShouldClose(window))
    {
        // Пернетақта оқиғаларын өңдеу
        glfwPollEvents();

        // Қазіргі уақыт
        float time =
            static_cast<float>(glfwGetTime());

        // =================================================
        // ҚОЗҒАЛАТЫН ФОН
        // =================================================

        float red =
            (std::sin(time * 2.0f) + 1.0f) / 2.0f;

        float green =
            (std::sin(time * 3.0f) + 1.0f) / 2.0f;

        float blue =
            (std::sin(time * 4.0f) + 1.0f) / 2.0f;

        // =================================================
        // SPACE ТЕКСЕРУ
        // =================================================

        if (spacePressed)
        {
            // SPACE басылып тұр
            // ФОН АҚ
            glClearColor(
                1.0f,
                1.0f,
                1.0f,
                1.0f
            );
        }
        else
        {
            // SPACE жіберілді
            // ФОН ҚАЙТА ӨЗГЕРЕДІ
            glClearColor(
                red,
                green,
                blue,
                1.0f
            );
        }

        // Экранды тазалау
        glClear(GL_COLOR_BUFFER_BIT);

        // =================================================
        // ТОРДЫ САЛУ
        // =================================================

        glUseProgram(shader);

        glBindVertexArray(vao);

        // Әр квадратқа 1 жоғарғы үшбұрыш
        // Барлығы: ROWS * COLS үшбұрыш

        glDrawArrays(
            GL_TRIANGLES,
            0,
            vertices.size() / 2
        );

        glBindVertexArray(0);

        // =================================================
        // Кадрды көрсету
        // =================================================

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
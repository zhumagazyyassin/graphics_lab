#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH = 1280;
const int HEIGHT = 720;

// SPACE басылып тұр ма?
bool spacePressed = false;

// Терезе өлшемі өзгерген кезде
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Пернетақта callback
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

int main()
{
    // GLFW іске қосу
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
        "Computer Graphics",
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

    // GLAD
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

    // FPS
    double lastTime = glfwGetTime();
    int frameCount = 0;

    // Негізгі цикл
    while (!glfwWindowShouldClose(window))
    {
        // Пернетақта оқиғаларын өңдеу
        glfwPollEvents();

        // Қазіргі уақыт
        float time = static_cast<float>(glfwGetTime());

        // =====================================
        // ҚОЗҒАЛАТЫН ФОН
        // =====================================

        float red =
            (std::sin(time * 2.0f) + 1.0f) / 2.0f;

        float green =
            (std::sin(time * 3.0f) + 1.0f) / 2.0f;

        float blue =
            (std::sin(time * 4.0f) + 1.0f) / 2.0f;

        // =====================================
        // SPACE ТЕКСЕРУ
        // =====================================

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

        // Кадрды көрсету
        glfwSwapBuffers(window);

        // =====================================
        // FPS
        // =====================================

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

    // Бағдарламаны жабу
    glfwTerminate();

    return 0;
}
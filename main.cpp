\#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

// 1-ТАПСЫРМА: Терезенің өлшемін 1280x720 ету
const int WIDTH  = 1280;
const int HEIGHT = 720;

// 3-ТАПСЫРМА: Tab басылғанын сақтайтын жаһандық айнымалы
bool isTabPressed = false;

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // 3-ТАПСЫРМА: Tab (Табуляция) басылғанын тексеру
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        isTabPressed = true;
    } else {
        isTabPressed = false;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Компьютерлік графика", nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);

    // 4-ТАПСЫРМА: VSync өшіру (0)
    glfwSwapInterval(0);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    // 4-ТАПСЫРМА: FPS есептеуге арналған айнымалылар
    double lastTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // 4-ТАПСЫРМА: FPS-ті секундына 1 рет консольге шығару
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }

        // --- Экранды тазалау ---
        if (isTabPressed) {
            // 3-ТАПСЫРМА: Tab басылғанда фон ақ түске айналады
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            // 2-ТАПСЫРМА: Фон түсінің өзгеру жылдамдығын арттыру
            float t = (float)glfwGetTime();
            float r = (std::sin(t * 3.0f) + 1.0f) * 0.5f * 0.3f;
            float g = (std::sin(t * 2.0f) + 1.0f) * 0.5f * 0.3f;
            glClearColor(r, g, 0.35f, 1.0f);
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


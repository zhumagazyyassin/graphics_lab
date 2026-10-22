
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

const int WIDTH = 900;
const int HEIGHT = 700;

const float RADIUS = 15.0f;
const float MOVE_SPEED = 350.0f;
const float GRAVITY = 1400.0f;
const float JUMP_POWER = 650.0f;

struct Platform {
    float x, y, w, h;
};

struct Vertex {
    float x, y;
    float r, g, b;
};

std::vector<Platform> platforms;
std::vector<Vertex> vertices;

float ballX = 150.0f;
float ballY = 81.0f;
float velocityY = 0.0f;

float cameraY = 0.0f;
float highestPlatformY = 66.0f;
float maxHeight = 81.0f;

bool grounded = true;
bool gameOver = false;

int score = 0;
int bestScore = 0;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader error: " << log << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool initRenderer() {
    GLuint vs = compileShader(
        GL_VERTEX_SHADER, vertexShaderSource
    );

    GLuint fs = compileShader(
        GL_FRAGMENT_SHADER, fragmentShaderSource
    );

    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint success = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        char log[1024] = {};
        glGetProgramInfoLog(
            shaderProgram, sizeof(log), nullptr, log
        );
        std::cerr << "Link error: " << log << '\n';
        return false;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(  
        0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, x))
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, r))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return true;
}

void addVertex(
    float x, float y,
    float r, float g, float b
) {
    float nx = 2.0f * x / WIDTH - 1.0f;
    float ny = 2.0f * y / HEIGHT - 1.0f;

    vertices.push_back({nx, ny, r, g, b});
}

void addTriangle(
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    float r, float g, float b
) {
    addVertex(x1, y1, r, g, b);
    addVertex(x2, y2, r, g, b);
    addVertex(x3, y3, r, g, b);
}

void addRect(
    float x, float y, float w, float h,
    float r, float g, float b
) {
    addTriangle(
        x, y, x + w, y, x + w, y + h,
        r, g, b
    );

    addTriangle(
        x, y, x + w, y + h, x, y + h,
        r, g, b
    );
}

void addCircle(float x, float y, float radius) {
    const int segments = 40;
    const float pi = 3.14159265358979323846f;

    for (int i = 0; i < segments; ++i) {
        float a1 = 2.0f * pi * i / segments;
        float a2 = 2.0f * pi * (i + 1) / segments;

        addTriangle(
            x, y,
            x + std::cos(a1) * radius,
            y + std::sin(a1) * radius,
            x + std::cos(a2) * radius,
            y + std::sin(a2) * radius,
            1.0f, 0.48f, 0.08f
        );
    }
}

void addDigit(
    int digit, float x, float y, float s
) {
    const int patterns[10] = {
        0b1111110,
        0b0110000,
        0b1101101,
        0b1111001,
        0b0110011,
        0b1011011,
        0b1011111,
        0b1110000,
        0b1111111,
        0b1111011
    };

    int p = patterns[digit];
    float t = s * 0.16f;
    float h = s * 1.6f;

    if (p & 64)
        addRect(x+t, y+h-t, s-2*t, t, 1,1,1);

    if (p & 32)
        addRect(x+s-t, y+h/2, t, h/2-t, 1,1,1);

    if (p & 16)
        addRect(x+s-t, y, t, h/2-t, 1,1,1);

    if (p & 8)
        addRect(x+t, y, s-2*t, t, 1,1,1);

    if (p & 4)
        addRect(x, y, t, h/2-t, 1,1,1);

    if (p & 2)
        addRect(x, y+h/2, t, h/2-t, 1,1,1);

    if (p & 1)
        addRect(
            x+t, y+h/2-t/2, s-2*t, t,
            1,1,1
        );
}
void addNumber(
    int number, float x, float y, float size
) {
    std::string digits = std::to_string(number);

    for (char c : digits) {
        addDigit(c - '0', x, y, size);
        x += size * 1.35f;
    }
}

// Generate platforms in different horizontal positions.
void addPlatform(float y) {
    float width = 65.0f + static_cast<float>(rand() % 86);

    float previousCenter =
        platforms.back().x + platforms.back().w / 2.0f;

    int type = rand() % 5;
    float shift = 0.0f;

    if (type == 0) {
        shift = static_cast<float>(rand() % 101 - 50);
    } else if (type == 1) {
        shift = static_cast<float>(rand() % 181 - 90);
    } else {
        shift = static_cast<float>(rand() % 401 - 200);
    }

    float center = previousCenter + shift;

    center = std::clamp(
        center,
        width / 2.0f + 5.0f,
        static_cast<float>(WIDTH) - width / 2.0f - 5.0f
    );

    platforms.push_back({
        center - width / 2.0f,
        y,
        width,
        16.0f
    });
}

void resetGame() {
    platforms.clear();

    ballX = 150.0f;
    ballY = 81.0f;
    velocityY = 0.0f;

    cameraY = 0.0f;
    highestPlatformY = 66.0f;
    maxHeight = 81.0f;

    grounded = true;
    gameOver = false;
    score = 0;

    platforms.push_back({
        70.0f, 50.0f, 180.0f, 16.0f
    });

    for (int i = 0; i < 10; ++i) {
        highestPlatformY +=
            55.0f + static_cast<float>(rand() % 61);

        addPlatform(highestPlatformY);
    }
}

void keyCallback(
    GLFWwindow* window,
    int key, int scancode, int action, int mods
) {
    if (key == GLFW_KEY_ESCAPE &&
        action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (gameOver && action == GLFW_PRESS &&
        (key == GLFW_KEY_R || key == GLFW_KEY_SPACE)) {
        resetGame();
    }
}

void updateGame(GLFWwindow* window, float dt) {
    if (gameOver) return;

    bool left =
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    bool right =
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    // Player controls horizontal direction.
    if (left) ballX -= MOVE_SPEED * dt;
    if (right) ballX += MOVE_SPEED * dt;

    ballX = std::clamp(
        ballX,
        RADIUS,
        static_cast<float>(WIDTH) - RADIUS
    );

    float oldBottom = ballY - RADIUS;

    // Automatic jump immediately after landing.
    if (grounded) {
        velocityY = JUMP_POWER;
        grounded = false;
    }

    // Gravity and vertical movement.
    velocityY -= GRAVITY * dt;
    ballY += velocityY * dt;

    // Check platform collision while falling.
    if (velocityY < 0.0f) {
        for (const auto& p : platforms) {
            bool overlaps =
                ballX + RADIUS > p.x &&
                ballX - RADIUS < p.x + p.w;

            float platformTop = p.y + p.h;
            float newBottom = ballY - RADIUS;

            if (overlaps &&
                oldBottom >= platformTop &&
                newBottom <= platformTop) {

                ballY = platformTop + RADIUS;
                velocityY = 0.0f;
                grounded = true;
                break;
            }
        }
    }

    // Score is based on the highest point reached.
    maxHeight = std::max(maxHeight, ballY);

    score = std::max(
        0,
        static_cast<int>((maxHeight - 81.0f) / 10.0f)
    );

    bestScore = std::max(bestScore, score);

    // Camera follows the player upward.
    if (ballY - cameraY > HEIGHT * 0.58f) {
        cameraY = ballY - HEIGHT * 0.58f;
    }

    // Create new platforms above the screen.
    while (highestPlatformY < cameraY + HEIGHT + 180.0f) {
        highestPlatformY +=
            55.0f + static_cast<float>(rand() % 61);

        addPlatform(highestPlatformY);
    }

    // Remove platforms far below.
    platforms.erase(
        std::remove_if(
            platforms.begin(),
            platforms.end(),
            [&](const Platform& p) {
                return p.y < cameraY - 100.0f;
            }
        ),
        platforms.end()
    );

    // Game over if the ball falls below the view.
    if (ballY + RADIUS < cameraY - 10.0f) {
        gameOver = true;
    }
}

void renderScene() {
    vertices.clear();

    for (const auto& p : platforms) {
        float screenY = p.y - cameraY;

        if (screenY > -30.0f &&
            screenY < HEIGHT + 30.0f) {
            addRect(
                p.x, screenY, p.w, p.h,
                0.12f, 0.88f, 0.30f
            );
        }
    }

    addCircle(ballX, ballY - cameraY, RADIUS);

    addNumber(
        score, 25.0f, HEIGHT - 65.0f, 17.0f
    );

    if (gameOver) {
        addRect(
            190, 260, 520, 130,
            0.16f, 0.18f, 0.25f
        );

        addRect(385, 300, 25, 50, 1, 1, 1);
        addRect(435, 300, 25, 50, 1, 1, 1);
    }

    glClearColor(0.055f, 0.075f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            vertices.size() * sizeof(Vertex)
        ),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(
        GL_TRIANGLES,
        0,
        static_cast<GLsizei>(vertices.size())
    );

    glBindVertexArray(0);
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Jumping Ball",
        nullptr, nullptr
    );

    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glViewport(0, 0, WIDTH, HEIGHT);

    if (!initRenderer()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    resetGame();

    double previousTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();

        float dt = static_cast<float>(
            now - previousTime
        );

        previousTime = now;
        dt = std::min(dt, 0.02f);

        updateGame(window, dt);

        std::string title =
            "Jumping Ball | Score: " +
            std::to_string(score) +
            " | Best: " + std::to_string(bestScore);

        if (gameOver) {
            title += " | GAME OVER - Press R";
        } else {
            title += " | A/D or arrows: move";
        }

        glfwSetWindowTitle(window, title.c_str());

        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// =====================================================================
//  10-апта: Толық еркін камера — тінтуірмен бағыт, дөңгелекпен зум
//  (Стильді түстер палитрасы және заманауи өңдеу)
// =====================================================================
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int gWidth = 1280, gHeight = 720;

// Камера параметрлері
const glm::vec3 CAM_START(0.0f, 0.0f, 3.0f);
glm::vec3 camPos   = CAM_START;
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp    = glm::vec3(0.0f, 1.0f, 0.0f);

const float YAW_START = -90.0f, FOV_START = 45.0f;
float yaw   = YAW_START;
float pitch = 0.0f;
float lastX = 640.0f;
float lastY = 360.0f;
bool  firstMouse = true;
float fov = FOV_START;
const float FOV_MAX = 90.0f;

const float sensitivity = 0.08f;  // Сәл жұмсақ әрі дәл сезімталдық
bool invertY    = false;
bool pitchLimit = true;
bool mouseOn    = true;

void onResize(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
    gWidth = w;
    gHeight = (h > 0) ? h : 1;
}

void updateFront() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(dir);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (!mouseOn) return;
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    if (invertY) yoffset = -yoffset;
    lastX = (float)xpos;
    lastY = (float)ypos;

    yaw   += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitchLimit) {
        if (pitch >  89.0f) pitch =  89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    } else {
        std::cout << "pitch = " << pitch << std::endl;
    }

    if (yaw >= 360.0f) yaw -= 360.0f;
    if (yaw <= -360.0f) yaw += 360.0f;

    updateFront();
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    fov -= (float)yoffset * 1.5f;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > FOV_MAX) fov = FOV_MAX;
}

bool pressedOnce(GLFWwindow* window, int key) {
    static bool prev[GLFW_KEY_LAST + 1] = {};
    bool now = glfwGetKey(window, key) == GLFW_PRESS;
    bool fired = now && !prev[key];
    prev[key] = now;
    return fired;
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    float speed = 3.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed *= 2.0f;

    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += right * speed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)      camPos += speed * camUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= speed * camUp;

    if (glm::length(camPos) > 25.0f) camPos = glm::normalize(camPos) * 25.0f;

    if (pressedOnce(window, GLFW_KEY_I)) invertY = !invertY;
    if (pressedOnce(window, GLFW_KEY_L)) pitchLimit = !pitchLimit;

    if (pressedOnce(window, GLFW_KEY_R)) { 
        camPos = CAM_START; yaw = YAW_START; pitch = 0.0f; fov = FOV_START; updateFront(); 
    }

    if (pressedOnce(window, GLFW_KEY_P)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "camPos   = (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n";
        std::cout << "camFront = (" << camFront.x << ", " << camFront.y << ", " << camFront.z << ")\n";
        std::cout << "yaw = " << std::setprecision(1) << yaw << "   pitch = " << pitch << "   fov = " << fov << "\n";
    }

    if (pressedOnce(window, GLFW_KEY_TAB)) {
        mouseOn = !mouseOn;
        glfwSetInputMode(window, GLFW_CURSOR, mouseOn ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (mouseOn) firstMouse = true;
    }
}

// Шейдерлер
const char* vsSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uModel, uView, uProj;
out vec3 vColor;
void main() { 
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0); 
    vColor = aColor; 
}
)";

const char* fsSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() { 
    // Жеңіл кірістірілген гамма-коррекция неонды әсер беру үшін
    vec3 result = pow(vColor, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0); 
}
)";

unsigned int makeShader() {
    auto compile = [](GLenum type, const char* src) {
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        int ok;
        glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
        if (!ok) { char log[512]; glGetShaderInfoLog(id, 512, nullptr, log); std::cerr << log << "\n"; }
        return id;
    };
    unsigned int vs = compile(GL_VERTEX_SHADER, vsSrc), fs = compile(GL_FRAGMENT_SHADER, fsSrc);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Заманауи киберпанк / неон түстер палитрасы бар куб генерациясы
unsigned int makeCube(int& indexCount) {
    struct Face { glm::vec3 n, u, v, color; };
    const Face faces[6] = {
        {{ 1, 0, 0}, {0, 0,-1}, {0, 1, 0}, {1.0f, 0.2f, 0.4f}}, // Қызғылт-неон
        {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1.0f, 0.5f, 0.1f}}, // Ашық сары-сары
        {{ 0, 1, 0}, {1, 0, 0}, {0, 0,-1}, {0.2f, 0.9f, 0.6f}}, // Минт-жасыл (үстіңгі)
        {{ 0,-1, 0}, {1, 0, 0}, {0, 0, 1}, {0.3f, 0.3f, 0.4f}}, // Қою күлгін-сұр (астыңғы)
        {{ 0, 0, 1}, {1, 0, 0}, {0, 1, 0}, {0.2f, 0.6f, 1.0f}}, // Көгілдір неон
        {{ 0, 0,-1}, {-1,0, 0}, {0, 1, 0}, {0.7f, 0.2f, 1.0f}}, // Күлгін
    };
    std::vector<float> vert;
    std::vector<unsigned int> idx;
    for (int f = 0; f < 6; f++) {
        unsigned int base = (unsigned int)(vert.size() / 6);
        for (int k = 0; k < 4; k++) {
            float su = (k == 0 || k == 3) ? -0.5f : 0.5f, sv = (k < 2) ? -0.5f : 0.5f;
            glm::vec3 p = faces[f].n * 0.5f + faces[f].u * su + faces[f].v * sv;
            glm::vec3 c = faces[f].color * (sv > 0 ? 1.1f : 0.8f); // Жарық градиенті
            vert.insert(vert.end(), { p.x, p.y, p.z, c.r, c.g, c.b });
        }
        idx.insert(idx.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }
    indexCount = (int)idx.size();
    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return vao;
}

int main() {
    if (!glfwInit()) { std::cerr << "GLFW іске қосылмады\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 10 - Modern Camera", nullptr, nullptr);
    if (!window) { std::cerr << "Терезе жасалмады\n"; glfwTerminate(); return -1; }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) { std::cerr << "GLAD жүктелмеді\n"; return -1; }
    
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    updateFront();

    int indexCount = 0;
    unsigned int cube = makeCube(indexCount);
    unsigned int shader = makeShader();
    int locModel = glGetUniformLocation(shader, "uModel");
    int locView  = glGetUniformLocation(shader, "uView");
    int locProj  = glGetUniformLocation(shader, "uProj");

    // Кубтардың орналасу позициялары
    const glm::vec3 cubePos[5] = { 
        {0.0f, 0.0f, 0.0f}, 
        {2.2f, 0.6f, -2.2f}, 
        {-2.0f, -0.4f, -1.5f}, 
        {1.5f, -1.0f, -3.8f}, 
        {-2.5f, 1.2f, -4.5f} 
    };

    float last = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime(), dt = std::min(now - last, 0.05f);
        last = now;

        processInput(window, dt);

        // Терең, қою түнгі көк түсті фон (Cyberpunk стиль)
        glClearColor(0.04f, 0.05f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glm::mat4 proj = glm::perspective(glm::radians(fov), (float)gWidth / (float)gHeight, 0.1f, 100.0f);
        
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(proj));

        glBindVertexArray(cube);
        for (int i = 0; i < 5; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos[i]);
            // Әр куб әр түрлі жылдамдықпен және бағытта айналады
            model = glm::rotate(model, now * (0.4f + i * 0.25f), glm::vec3(0.2f, 1.0f, 0.1f * i));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        }

        // Терезе тақырыбында параметрлерді көрсету
        char buf[128];
        snprintf(buf, sizeof(buf), "Yaw: %.1f | Pitch: %.1f | FOV: %.1f%s%s", yaw, pitch, fov,
                 invertY ? " [InvertY]" : "", pitchLimit ? "" : " [NoLimit]");
        static std::string lastTitle;
        if (lastTitle != buf) { glfwSetWindowTitle(window, buf); lastTitle = buf; }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
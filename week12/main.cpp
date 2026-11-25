#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#ifndef ASSET_DIR
#define ASSET_DIR "../assets"
#endif

int gWidth = 800, gHeight = 600;

glm::vec3 camPos(0.0f, 0.0f, 3.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
float fov = 45.0f;

bool firstMouse = true;
bool invertY = false;
bool wireframe = false;

GLuint texture1 = 0;
GLuint texture2 = 0;
GLenum wrapMode = GL_REPEAT;
GLenum magFilter = GL_LINEAR;
GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;

float blend = 0.5f;

const char* vertexSrc = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;
}
)";

const char* fragmentSrc = R"(#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float blend;

void main() {
    FragColor = mix(texture(texture1, vTexCoord), texture(texture2, vTexCoord), blend);
}
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader error: " << log << '\n';
    }
    return shader;
}

GLuint createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program link error: " << log << '\n';
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void applyTextureParametersToId(GLuint texId) {
    if (!texId) return;
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    const float border[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
}

void applyTextureParameters() {
    applyTextureParametersToId(texture1);
    applyTextureParametersToId(texture2);
}

GLuint loadTexture(const char* path) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    int w = 0, h = 0, n = 0;
    unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (data) {
        GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << path << " " << w << "x" << h << " channels: " << n << '\n';
    } else {
        std::cerr << "Failed to load texture: " << path << '\n';
        glDeleteTextures(1, &id);
        id = 0;
    }

    stbi_image_free(data);
    return id;
}

void framebufferCallback(GLFWwindow*, int width, int height) {
    gWidth = width;
    gHeight = (height > 0) ? height : 1;
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    if (invertY) {
        yoffset = -yoffset;
    }

    constexpr float sensitivity = 0.1f;
    yaw += xoffset * sensitivity;
    pitch = std::clamp(pitch + yoffset * sensitivity, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction.y = std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    camFront = glm::normalize(direction);
}

void scrollCallback(GLFWwindow*, double, double yoffset) {
    fov = std::clamp(fov - static_cast<float>(yoffset), 1.0f, 90.0f);
}

bool justPressed(GLFWwindow* window, int key, bool& prevState) {
    const bool down = (glfwGetKey(window, key) == GLFW_PRESS);
    const bool triggered = (down && !prevState);
    prevState = down;
    return triggered;
}

void setWrap(GLenum mode, const char* name) {
    wrapMode = mode;
    applyTextureParameters();
    std::cout << "Wrap: " << name << '\n';
}

void processActions(GLFWwindow* window) {
    static bool p1 = false, p2 = false, p3 = false, p4 = false;
    static bool pN = false, pL = false, pM = false, pI = false;
    static bool pTab = false, pR = false, pE = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (justPressed(window, GLFW_KEY_1, p1)) setWrap(GL_REPEAT, "GL_REPEAT");
    if (justPressed(window, GLFW_KEY_2, p2)) setWrap(GL_MIRRORED_REPEAT, "GL_MIRRORED_REPEAT");
    if (justPressed(window, GLFW_KEY_3, p3)) setWrap(GL_CLAMP_TO_EDGE, "GL_CLAMP_TO_EDGE");
    if (justPressed(window, GLFW_KEY_4, p4)) setWrap(GL_CLAMP_TO_BORDER, "GL_CLAMP_TO_BORDER");

    if (justPressed(window, GLFW_KEY_N, pN)) {
        magFilter = GL_NEAREST;
        applyTextureParameters();
        std::cout << "MAG: GL_NEAREST\n";
    }
    if (justPressed(window, GLFW_KEY_L, pL)) {
        magFilter = GL_LINEAR;
        applyTextureParameters();
        std::cout << "MAG: GL_LINEAR\n";
    }
    if (justPressed(window, GLFW_KEY_M, pM)) {
        minFilter = (minFilter == GL_LINEAR) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        applyTextureParameters();
        std::cout << "MIN: "
                  << ((minFilter == GL_LINEAR) ? "GL_LINEAR" : "GL_LINEAR_MIPMAP_LINEAR")
                  << '\n';
    }
    if (justPressed(window, GLFW_KEY_I, pI)) {
        invertY = !invertY;
        std::cout << "Invert Y: " << invertY << '\n';
    }
    if (justPressed(window, GLFW_KEY_TAB, pTab)) {
        wireframe = !wireframe;
        std::cout << "Wireframe: " << wireframe << '\n';
    }
    if (justPressed(window, GLFW_KEY_R, pR)) {
        camPos = glm::vec3(0.0f, 0.0f, 3.0f);
        camFront = glm::vec3(0.0f, 0.0f, -1.0f);
        yaw = -90.0f;
        pitch = 0.0f;
        fov = 45.0f;
        blend = 0.5f;
        std::cout << "Reset camera & blend\n";
    }
    if (justPressed(window, GLFW_KEY_E, pE)) {
        while (glGetError() != GL_NO_ERROR) {}
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        GLenum error = glGetError();
        std::cout << "Intentional invalid MAG filter, glGetError = 0x"
                  << std::hex << error << std::dec << '\n';
        applyTextureParameters();
    }
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 12 - Multiple Textures", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }

    const float vertices[] = {
        -0.5f,-0.5f,-0.5f, 0.0f,0.0f,  -0.5f, 0.5f,-0.5f, 0.0f,1.0f,   0.5f, 0.5f,-0.5f, 1.0f,1.0f,
         0.5f, 0.5f,-0.5f, 1.0f,1.0f,   0.5f,-0.5f,-0.5f, 1.0f,0.0f,  -0.5f,-0.5f,-0.5f, 0.0f,0.0f,

        -0.5f,-0.5f, 0.5f, 0.0f,0.0f,   0.5f,-0.5f, 0.5f, 1.0f,0.0f,   0.5f, 0.5f, 0.5f, 1.0f,1.0f,
         0.5f, 0.5f, 0.5f, 1.0f,1.0f,  -0.5f, 0.5f, 0.5f, 0.0f,1.0f,  -0.5f,-0.5f, 0.5f, 0.0f,0.0f,

        -0.5f,-0.5f,-0.5f, 0.0f,0.0f,  -0.5f,-0.5f, 0.5f, 1.0f,0.0f,  -0.5f, 0.5f, 0.5f, 1.0f,1.0f,
        -0.5f, 0.5f, 0.5f, 1.0f,1.0f,  -0.5f, 0.5f,-0.5f, 0.0f,1.0f,  -0.5f,-0.5f,-0.5f, 0.0f,0.0f,

         0.5f,-0.5f,-0.5f, 0.0f,0.0f,   0.5f, 0.5f,-0.5f, 0.0f,1.0f,   0.5f, 0.5f, 0.5f, 1.0f,1.0f,
         0.5f, 0.5f, 0.5f, 1.0f,1.0f,   0.5f,-0.5f, 0.5f, 1.0f,0.0f,   0.5f,-0.5f,-0.5f, 0.0f,0.0f,

        -0.5f,-0.5f,-0.5f, 0.0f,0.0f,   0.5f,-0.5f,-0.5f, 1.0f,0.0f,   0.5f,-0.5f, 0.5f, 1.0f,1.0f,
         0.5f,-0.5f, 0.5f, 1.0f,1.0f,  -0.5f,-0.5f, 0.5f, 0.0f,1.0f,  -0.5f,-0.5f,-0.5f, 0.0f,0.0f,

        -0.5f, 0.5f,-0.5f, 0.0f,0.0f,  -0.5f, 0.5f, 0.5f, 0.0f,1.0f,   0.5f, 0.5f, 0.5f, 1.0f,1.0f,
         0.5f, 0.5f, 0.5f, 1.0f,1.0f,   0.5f, 0.5f,-0.5f, 1.0f,0.0f,  -0.5f, 0.5f,-0.5f, 0.0f,0.0f
    };

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint program = createProgram();
    const GLint modelLoc = glGetUniformLocation(program, "model");
    const GLint viewLoc = glGetUniformLocation(program, "view");
    const GLint projectionLoc = glGetUniformLocation(program, "projection");
    const GLint blendLoc = glGetUniformLocation(program, "blend");

    const std::string tex1Path = std::string(ASSET_DIR) + "/container.png";
    const std::string tex2Path = std::string(ASSET_DIR) + "/texture2.png";

    texture1 = loadTexture(tex1Path.c_str());
    if (!texture1) texture1 = loadTexture("../../assets/container.png");

    texture2 = loadTexture(tex2Path.c_str());
    if (!texture2) texture2 = loadTexture("../../assets/texture2.png");

    if (!texture1 || !texture2) {
        if (texture1) glDeleteTextures(1, &texture1);
        if (texture2) glDeleteTextures(1, &texture2);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(program);
        glfwTerminate();
        return -1;
    }

    applyTextureParameters();

    const glm::vec3 positions[] = {
        {0.0f, 0.0f, 0.0f}, {2.0f, 0.5f, -2.0f}, {-1.8f, -0.3f, -1.0f},
        {1.2f, -0.8f, -3.5f}, {-2.2f, 1.0f, -4.0f},
        {-5.0f, 0.0f, -38.0f}, {-3.0f, 1.0f, -40.0f}, {-1.0f, -1.0f, -42.0f},
        {1.0f, 0.5f, -39.0f}, {3.0f, -0.5f, -41.0f}, {5.0f, 1.0f, -40.0f}
    };

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "texture1"), 0);
    glUniform1i(glGetUniformLocation(program, "texture2"), 1);

    std::cout << "Keys: UP/DOWN blend, 1-4 wrap, N/L MAG, M mipmap, I invertY, Tab wireframe, R reset, E error test\n";

    float lastFrame = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        const float now = static_cast<float>(glfwGetTime());
        const float dt = now - lastFrame;
        lastFrame = now;

        processActions(window);

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            blend += 0.5f * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            blend -= 0.5f * dt;
        }
        if (blend > 1.0f) blend = 1.0f;
        if (blend < 0.0f) blend = 0.0f;

        float speed = 2.5f * dt;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed *= 2.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += speed * camFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= speed * camFront;

        const glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= speed * right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += speed * right;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camPos += speed * camUp;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= speed * camUp;

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = (gHeight > 0) ? (static_cast<float>(gWidth) / gHeight) : 1.0f;
        const glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 120.0f);
        const glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);

        glUseProgram(program);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(blendLoc, blend);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glBindVertexArray(vao);

        const int cubeCount = static_cast<int>(sizeof(positions) / sizeof(positions[0]));
        for (int i = 0; i < cubeCount; ++i) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), positions[i]);
            model = glm::rotate(model, now * (0.25f + i * 0.04f), glm::vec3(0.3f, 1.0f, 0.1f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
}
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH  = 1280;
const int HEIGHT = 720;

bool wireframe = false;
bool prevKeys[GLFW_KEY_LAST + 1] = { false };

// Пернені бір рет басуды тексеру функциясы
bool pressedOnce(GLFWwindow* window, int key) {
    bool now = glfwGetKey(window, key) == GLFW_PRESS;
    bool fired = now && !prevKeys[key];
    prevKeys[key] = now;
    return fired;
}

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (pressedOnce(window, GLFW_KEY_TAB)) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
}

// Шейдерді жасау функциясы
unsigned int makeShader(const char* vsSrc, const char* fsSrc) {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    int ok;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(vs, 1024, nullptr, log); std::cerr << log << "\n"; }

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(fs, 1024, nullptr, log); std::cerr << log << "\n"; }

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(prog, 1024, nullptr, log); std::cerr << log << "\n"; }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform vec2 uOffset;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPos.xy + uOffset, aPos.z, 1.0);
    vColor = aColor;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() { FragColor = vec4(vColor, 1.0); }
)";

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW iske qosylmady\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2D Oйын-саxна: Жұлдыз бен Алтыбұрыш", nullptr, nullptr);
    if (!window) {
        std::cerr << "Tereze zhasalmady.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";
    std::cout << "BASQARU: TAB - wireframe, ESC - shygu\n";

    const int STRIDE = 6 * sizeof(float);
    float aspect = (float)WIDTH / (float)HEIGHT;

    // =================================================================
    // 1. ЖҰЛДЫЗ ФИГУРАСЫ (Бұрынғы шардың орнына - Жарқыраған жұлдыз/көпбұрыш)
    // =================================================================
    const int starSegments = 16; // Ерекше геометриялық форма үшін
    float starVertices[(starSegments + 1) * 6];
    unsigned int starIndices[starSegments * 3];

    // Центр (Өте ашық аппақ-сары жарқырау)
    starVertices[0] = 0.0f;
    starVertices[1] = 0.0f;
    starVertices[2] = 0.0f;
    starVertices[3] = 1.0f;
    starVertices[4] = 1.0f;
    starVertices[5] = 0.8f;

    const float PI = 3.14159265f;
    float outerRadius = 0.15f;
    float innerRadius = 0.08f;

    for (int i = 0; i < starSegments; i++) {
        float theta = 2.0f * PI * float(i) / float(starSegments);
        // Жұлдыз тәрізді форма беру үшін радиусты алмастырып отырамыз
        float currentRadius = (i % 2 == 0) ? outerRadius : innerRadius;
        
        int idx = (i + 1) * 6;
        starVertices[idx + 0] = (currentRadius * std::cos(theta)) / aspect;
        starVertices[idx + 1] = currentRadius * std::sin(theta);
        starVertices[idx + 2] = 0.0f;
        
        // Қанық отты қызыл және ашық неон көк градиент түстері
        if (i % 2 == 0) {
            starVertices[idx + 3] = 1.0f; // Қызыл
            starVertices[idx + 4] = 0.1f; // Жасыл
            starVertices[idx + 5] = 0.3f; // Көк
        } else {
            starVertices[idx + 3] = 0.1f; 
            starVertices[idx + 4] = 0.9f; 
            starVertices[idx + 5] = 1.0f; // Неон көк
        }
    }

    for (int i = 0; i < starSegments; i++) {
        starIndices[i * 3 + 0] = 0;
        starIndices[i * 3 + 1] = i + 1;
        starIndices[i * 3 + 2] = (i + 1) % starSegments + 1;
    }

    unsigned int vaoStar, vboStar, eboStar;
    glGenVertexArrays(1, &vaoStar);
    glGenBuffers(1, &vboStar);
    glGenBuffers(1, &eboStar);

    glBindVertexArray(vaoStar);
    glBindBuffer(GL_ARRAY_BUFFER, vboStar);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboStar);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(starIndices), starIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // =================================================================
    // 2. АЛТЫБҰРЫШ ФИГУРАСЫ (Бұрынғы төртбұрыштың орнына - Алтыбұрыш / Hexagon)
    // =================================================================
    const int hexSegments = 6;
    float hexVertices[(hexSegments + 1) * 6];
    unsigned int hexIndices[hexSegments * 3];

    // Алтыбұрыштың центрі
    hexVertices[0] = 0.0f;
    hexVertices[1] = 0.0f;
    hexVertices[2] = 0.0f;
    hexVertices[3] = 0.2f;
    hexVertices[4] = 1.0f;
    hexVertices[5] = 0.2f; // Ашық неон жасыл центр

    float hexRadius = 0.13f;
    for (int i = 0; i < hexSegments; i++) {
        float theta = 2.0f * PI * float(i) / float(hexSegments);
        int idx = (i + 1) * 6;
        hexVertices[idx + 0] = (hexRadius * std::cos(theta)) / aspect;
        hexVertices[idx + 1] = hexRadius * std::sin(theta);
        hexVertices[idx + 2] = 0.0f;

        // Әр бұрышына әдемі күлгін, сары және көк түстер береміз
        if (i % 3 == 0) {
            hexVertices[idx + 3] = 0.8f; hexVertices[idx + 4] = 0.0f; hexVertices[idx + 5] = 1.0f; // Күлгін
        } else if (i % 3 == 1) {
            hexVertices[idx + 3] = 1.0f; hexVertices[idx + 4] = 0.8f; hexVertices[idx + 5] = 0.0f; // Алтын сары
        } else {
            hexVertices[idx + 3] = 0.0f; hexVertices[idx + 4] = 1.0f; hexVertices[idx + 5] = 0.9f; // Көгілдір
        }
    }

    for (int i = 0; i < hexSegments; i++) {
        hexIndices[i * 3 + 0] = 0;
        hexIndices[i * 3 + 1] = i + 1;
        hexIndices[i * 3 + 2] = (i + 1) % hexSegments + 1;
    }

    unsigned int vaoHex, vboHex, eboHex;
    glGenVertexArrays(1, &vaoHex);
    glGenBuffers(1, &vboHex);
    glGenBuffers(1, &eboHex);

    glBindVertexArray(vaoHex);
    glBindBuffer(GL_ARRAY_BUFFER, vboHex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hexVertices), hexVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboHex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hexIndices), hexIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // -----------------------------------------------------------------
    unsigned int shader = makeShader(vertexSrc, fragmentSrc);
    int locOffset = glGetUniformLocation(shader, "uOffset");

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        float time = (float)glfwGetTime();

        // --- ЖЕРДЕГІ ҚОЗҒАЛЫС ЖӘНЕ ТАЙМИНГ (Дәл синхрондалған қатынас) ---
        float groundY = -0.5f;

        const float quadSpeed = 1.1f;
        const float ballSpeed = 3.0f * quadSpeed;   // = 3.3

        // 1. Жұлдыз: орта нүктеде тұрып, жоғары-төмен ыршиды
        float bounceHeight = 0.65f;
        float starY = groundY + outerRadius + bounceHeight * std::abs(std::sin(time * ballSpeed));
        float starX = 0.0f;

        // 2. Алтыбұрыш: x = 0 нүктесінен өткен сайын жұлдыз дәл ең биік нүктесінде болады
        float hexX = 1.4f * std::cos(time * quadSpeed);
        float hexY = groundY + hexRadius;

        // --- СЫЗУ (Терең қара-күлгін неон фон) ---
        glClearColor(0.06f, 0.03f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        // Алтыбұрышты сызу
        glBindVertexArray(vaoHex);
        glUniform2f(locOffset, hexX, hexY);
        glDrawElements(GL_TRIANGLES, hexSegments * 3, GL_UNSIGNED_INT, 0);

        // Жұлдызды сызу
        glBindVertexArray(vaoStar);
        glUniform2f(locOffset, starX, starY);
        glDrawElements(GL_TRIANGLES, starSegments * 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Ресурстарды тазалау ---
    glDeleteVertexArrays(1, &vaoStar);
    glDeleteVertexArrays(1, &vaoHex);
    glDeleteBuffers(1, &vboStar);
    glDeleteBuffers(1, &vboHex);
    glDeleteBuffers(1, &eboStar);
    glDeleteBuffers(1, &eboHex);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
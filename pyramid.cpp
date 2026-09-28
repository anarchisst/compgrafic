// =====================================================================
//  ПИРАМИДА — үшбұрыштардан құралған пирамида
//
//  Қабат санын консольден енгізесің. N қабат болса:
//      1-қабатта 1 үшбұрыш, 2-қабатта 2, ..., N-қабатта N үшбұрыш.
//      Барлығы N*(N+1)/2 үшбұрыш.
//
//  main.cpp-ға тимейді — бөлек бағдарлама (CMake-те "pyramid" деген атпен).
//
//  Пернелер:
//      W (басып тұр) — wireframe
//      Esc           — шығу
// =====================================================================

#include <glad/gl.h>      // МІНДЕТТІ: glad әрқашан GLFW-дан БҰРЫН
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

const int WIDTH  = 800;
const int HEIGHT = 800;

// ---------------------------------------------------------------------
//  Шейдерлер (2-аптадағыдай ең қарапайым күйде)
// ---------------------------------------------------------------------
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0, 0.5, 0.2, 1.0); }
)";

bool wireframe = false;

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    wireframe = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
}

// ---------------------------------------------------------------------
//  Пирамиданың вершиналарын есептейді.
//
//  Үлкен үшбұрыш: төбесі (0, top), табаны y = bottom, ені = width.
//  Оны N қабатқа бөлеміз. Әр қабаттың биіктігі h = (top - bottom) / N,
//  әр кішкентай үшбұрыштың табанының ені w = width / N.
//
//  r-ші қабатта (r = 0, 1, 2, ...) r+1 үшбұрыш бар. Олар қатарынан,
//  табандарының бұрыштары түйісіп тұрады. Қатардың сол жақ шеті:
//      xStart = -(r + 1) * w / 2
// ---------------------------------------------------------------------
std::vector<float> buildPyramid(int levels) {
    std::vector<float> v;
    v.reserve(levels * (levels + 1) / 2 * 9);   // әр үшбұрышқа 3 вершина * 3 сан

    const float top    =  0.9f;
    const float bottom = -0.9f;
    const float width  =  1.8f;

    const float h = (top - bottom) / levels;    // қабат биіктігі
    const float w = width / levels;             // кіші үшбұрыштың табан ені

    for (int r = 0; r < levels; r++) {          // қабаттар (жоғарыдан төмен)
        float yTop    = top - r * h;            // қабаттың жоғарғы сызығы
        float yBottom = top - (r + 1) * h;      // қабаттың төменгі сызығы
        float xStart  = -(r + 1) * w / 2.0f;    // қатардың сол шеті

        for (int k = 0; k <= r; k++) {          // қабаттағы үшбұрыштар (r+1 дана)
            float xLeft  = xStart + k * w;
            float xRight = xLeft + w;
            float xMid   = xLeft + w / 2.0f;

            // сол жақ төменгі бұрыш
            v.push_back(xLeft);  v.push_back(yBottom); v.push_back(0.0f);
            // оң жақ төменгі бұрыш
            v.push_back(xRight); v.push_back(yBottom); v.push_back(0.0f);
            // төбесі
            v.push_back(xMid);   v.push_back(yTop);    v.push_back(0.0f);
        }
    }
    return v;
}

int main() {

    // -----------------------------------------------------------------
    //  Қабат санын енгізу (терезе ашылмай тұрып)
    // -----------------------------------------------------------------
    int levels = 0;
    while (levels < 1 || levels > 100) {
        std::cout << "Неше қабат болсын? (1..100): ";
        if (!(std::cin >> levels)) {
            if (std::cin.eof()) return 1;       // енгізу жабылса — шығамыз
            std::cin.clear();                   // сан емес нәрсе жазылса —
            std::cin.ignore(10000, '\n');       // тазалап, қайта сұраймыз
            levels = 0;
        }
    }

    // -----------------------------------------------------------------
    //  GLFW, терезе, GLAD (2-аптадағыдай)
    // -----------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    int triangleCount = levels * (levels + 1) / 2;
    std::string title = "Пирамида: " + std::to_string(levels) + " қабат, "
                      + std::to_string(triangleCount) + " үшбұрыш";

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(),
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    // -----------------------------------------------------------------
    //  Пирамиданы есептеп, GPU-ға жүктейміз (VAO + VBO, 2-аптадағыдай)
    // -----------------------------------------------------------------
    std::vector<float> vertices = buildPyramid(levels);
    int vertexCount = (int)(vertices.size() / 3);

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // vertices — vector, сондықтан sizeof емес, size() * sizeof(float)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Шейдерлерді компиляциялау
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, nullptr);
    glCompileShader(fs);

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // -----------------------------------------------------------------
    //  Негізгі цикл
    // -----------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);   // барлық үшбұрыш бір шақырумен

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

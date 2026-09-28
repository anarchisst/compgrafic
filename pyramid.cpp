#include <glad/gl.h>      
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

const int WIDTH  = 800;
const int HEIGHT = 800;

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


std::vector<float> buildPyramid(int levels) {
    std::vector<float> v;
    v.reserve(levels * (levels + 1) / 2 * 9);   

    const float top    =  0.9f;
    const float bottom = -0.9f;
    const float width  =  1.8f;

    const float h = (top - bottom) / levels;    
    const float w = width / levels;             

    for (int r = 0; r < levels; r++) {          
        float yTop    = top - r * h;            
        float yBottom = top - (r + 1) * h;      
        float xStart  = -(r + 1) * w / 2.0f;    

        for (int k = 0; k <= r; k++) {          
            float xLeft  = xStart + k * w;
            float xRight = xLeft + w;
            float xMid   = xLeft + w / 2.0f;

            
            v.push_back(xLeft);  v.push_back(yBottom); v.push_back(0.0f);
            
            v.push_back(xRight); v.push_back(yBottom); v.push_back(0.0f);
            
            v.push_back(xMid);   v.push_back(yTop);    v.push_back(0.0f);
        }
    }
    return v;
}

int main() {

    
    int levels = 0;
    while (levels < 1 || levels > 100) {
        std::cout << "Неше қабат болсын? (1..100): ";
        if (!(std::cin >> levels)) {
            if (std::cin.eof()) return 1;       
            std::cin.clear();                   
            std::cin.ignore(10000, '\n');       
            levels = 0;
        }
    }

    
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

   
    std::vector<float> vertices = buildPyramid(levels);
    int vertexCount = (int)(vertices.size() / 3);

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
   
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

  
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

    
    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);   

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

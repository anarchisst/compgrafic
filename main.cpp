
#include <glad/gl.h>      
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>


const int WIDTH  = 1280;   
const int HEIGHT = 720;


const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;

void main() { FragColor = vec4(0.2, 0.9, 0.3, 1.0); }
)";


bool   whiteBackground = false;         
GLenum drawMode        = GL_TRIANGLES;  
bool   wireframe       = false;         


void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    
    whiteBackground = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) drawMode = GL_TRIANGLES;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) drawMode = GL_LINE_LOOP;

    
    wireframe = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
}


int main() {

    
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

    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
                                          "Компьютерлік графика",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады. Видеокарта OpenGL 3.3-ті "
                     "қолдамауы мүмкін.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);              
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(0);                        

   
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";


    
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
       
        -0.5f,  0.3f, 0.0f,
         0.5f,  0.3f, 0.0f,
         0.0f, -0.7f, 0.0f
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);                                  
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                      
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),          
                 vertices, GL_STATIC_DRAW);

    
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


   
    double lastFpsTime = glfwGetTime();   
    int    frameCount  = 0;

    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        
        frameCount++;
        double now = glfwGetTime();
        if (now - lastFpsTime >= 1.0) {
            std::cout << "FPS: " << frameCount << "\n";
            frameCount  = 0;
            lastFpsTime = now;
        }

        // --- Экранды тазалау ---
        if (whiteBackground) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);           
        } else {
            float t = (float)glfwGetTime();
           
            float r = (std::sin(t * 2.0f) + 1.0f) * 0.5f * 0.3f;
            float g = (std::sin(t * 1.3f) + 1.0f) * 0.5f * 0.3f;
            glClearColor(r, g, 0.35f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(drawMode, 0, 6);                        

        glfwSwapBuffers(window);   
        glfwPollEvents();          
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

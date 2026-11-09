#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>

const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 700;
int gWidth = WIDTH;
int gHeight = HEIGHT;

glm::vec3 camPos(0.0f, 0.0f, 3.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

float fov = 45.0f;
const float FOV_MIN = 1.0f;
const float FOV_MAX = 90.0f;

bool invertY = false;
bool iPressedLast = false;
bool pPressedLast = false;

void updateCamFront()
{
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(dir);
}

void resetCamera()
{
    camPos = glm::vec3(0.0f, 0.0f, 3.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    fov = 45.0f;
    updateCamFront();
}

void printCameraState()
{
    std::cout << std::fixed << std::setprecision(2)
              << "camPos   = (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n"
              << "camFront = (" << camFront.x << ", " << camFront.y << ", " << camFront.z << ")\n"
              << std::setprecision(1)
              << "yaw = " << yaw << "   pitch = " << pitch << "   fov = " << fov << '\n'
              << std::setprecision(7)
              << "length = " << glm::length(camFront) << "\n\n";
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0) {
        return;
    }
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    if (invertY) {
        yoffset = -yoffset;
    }
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    if (yaw > 360.0f) {
        yaw -= 360.0f;
        std::cout << "yaw 360-тан асты, жаңа мәні: " << yaw << '\n';
    }
    if (yaw < -360.0f) {
        yaw += 360.0f;
        std::cout << "yaw -360-тан кіші болды, жаңа мәні: " << yaw << '\n';
    }

    updateCamFront();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < FOV_MIN) fov = FOV_MIN;
    if (fov > FOV_MAX) fov = FOV_MAX;
}

void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float camSpeed = 2.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camSpeed *= 2.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camPos += camSpeed * camFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camPos -= camSpeed * camFront;
    }

    const glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camPos -= camSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camPos += camSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camPos += camSpeed * camUp;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camPos -= camSpeed * camUp;
    }

    const bool iNow = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    if (iNow && !iPressedLast) {
        invertY = !invertY;
        std::cout << "invertY = " << (invertY ? "true" : "false") << '\n';
    }
    iPressedLast = iNow;

    const bool pNow = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pNow && !pPressedLast) {
        printCameraState();
    }
    pPressedLast = pNow;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        resetCamera();
    }
}

const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentSource = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";

bool compileShader(unsigned int shader)
{
    glCompileShader(shader);

    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader қатесі: " << log << '\n';
    }

    return ok != 0;
}

int main()
{
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Week 10 - Mouse look / FOV zoom",
        nullptr,
        nullptr
    );
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }

    glfwGetFramebufferSize(window, &gWidth, &gHeight);
    glViewport(0, 0, gWidth, gHeight);
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    const float vertices[] = {
        -0.5f, -0.5f,  0.5f, 1,    .2f,  .2f,
         0.5f, -0.5f,  0.5f, 1,    .2f,  .2f,
         0.5f,  0.5f,  0.5f, 1,    .2f,  .2f,
         0.5f,  0.5f,  0.5f, 1,    .2f,  .2f,
        -0.5f,  0.5f,  0.5f, 1,    .2f,  .2f,
        -0.5f, -0.5f,  0.5f, 1,    .2f,  .2f,

        -0.5f, -0.5f, -0.5f, .2f, .4f,  1,
         0.5f,  0.5f, -0.5f, .2f, .4f,  1,
         0.5f, -0.5f, -0.5f, .2f, .4f,  1,
         0.5f,  0.5f, -0.5f, .2f, .4f,  1,
        -0.5f, -0.5f, -0.5f, .2f, .4f,  1,
        -0.5f,  0.5f, -0.5f, .2f, .4f,  1,

        -0.5f,  0.5f,  0.5f, .2f, .9f, .5f,
        -0.5f,  0.5f, -0.5f, .2f, .9f, .5f,
        -0.5f, -0.5f, -0.5f, .2f, .9f, .5f,
        -0.5f, -0.5f, -0.5f, .2f, .9f, .5f,
        -0.5f, -0.5f,  0.5f, .2f, .9f, .5f,
        -0.5f,  0.5f,  0.5f, .2f, .9f, .5f,

         0.5f,  0.5f,  0.5f, 1,    .8f,  .2f,
         0.5f, -0.5f, -0.5f, 1,    .8f,  .2f,
         0.5f,  0.5f, -0.5f, 1,    .8f,  .2f,
         0.5f, -0.5f, -0.5f, 1,    .8f,  .2f,
         0.5f,  0.5f,  0.5f, 1,    .8f,  .2f,
         0.5f, -0.5f,  0.5f, 1,    .8f,  .2f,

        -0.5f,  0.5f, -0.5f, .8f, .3f,  1,
         0.5f,  0.5f,  0.5f, .8f, .3f,  1,
         0.5f,  0.5f, -0.5f, .8f, .3f,  1,
         0.5f,  0.5f,  0.5f, .8f, .3f,  1,
        -0.5f,  0.5f, -0.5f, .8f, .3f,  1,
        -0.5f,  0.5f,  0.5f, .8f, .3f,  1,

        -0.5f, -0.5f, -0.5f, .2f, .9f, .9f,
         0.5f, -0.5f, -0.5f, .2f, .9f, .9f,
         0.5f, -0.5f,  0.5f, .2f, .9f, .9f,
         0.5f, -0.5f,  0.5f, .2f, .9f, .9f,
        -0.5f, -0.5f,  0.5f, .2f, .9f, .9f,
        -0.5f, -0.5f, -0.5f, .2f, .9f, .9f
    };

    unsigned int vao;
    unsigned int vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const int stride = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vertexSource, nullptr);
    glShaderSource(fs, 1, &fragmentSource, nullptr);
    if (!compileShader(vs) || !compileShader(fs)) {
        return -1;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Shader program link қатесі\n";
        return -1;
    }

    const int modelLoc = glGetUniformLocation(program, "model");
    const int viewLoc = glGetUniformLocation(program, "view");
    const int projectionLoc = glGetUniformLocation(program, "projection");
    const glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,   0.0f),
        glm::vec3( 2.2f,  1.2f,  -4.0f),
        glm::vec3(-2.6f, -0.8f,  -3.5f),
        glm::vec3(-1.5f,  2.5f,  -6.0f),
        glm::vec3( 3.0f, -1.5f,  -7.5f),
        glm::vec3( 0.5f,  3.5f,  -2.5f),
        glm::vec3(-4.0f,  1.0f,  -9.0f),
        glm::vec3( 1.2f, -2.5f,  -5.0f),
    };
    const int cubeCount = sizeof(cubePositions) / sizeof(cubePositions[0]);

    float lastFrame = float(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        const float now = float(glfwGetTime());
        const float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        char buf[128];
        snprintf(buf, sizeof(buf), "Week 10 | yaw=%.1f  pitch=%.1f  fov=%.1f%s",
                 yaw, pitch, fov, invertY ? "  [invert Y]" : "");
        glfwSetWindowTitle(window, buf);

        glClearColor(.075f, .09f, .14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        const glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)gWidth / (float)gHeight,
            0.1f,
            100.0f
        );

        glUseProgram(program);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(vao);
        for (int i = 0; i < cubeCount; i++) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(25.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}

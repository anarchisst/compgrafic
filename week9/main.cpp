#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 700;

glm::vec3 camPos(0.0f, 0.0f, 3.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
bool followCameraDirection = true;

void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    const float speed = 2.5f * dt;
    const float moveSpeed = speed *
        (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 2.0f : 1.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camPos += moveSpeed * camFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camPos -= moveSpeed * camFront;
    }

    const glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camPos -= moveSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camPos += moveSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camPos += moveSpeed * camUp;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camPos -= moveSpeed * camUp;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camPos = glm::vec3(0.0f, 0.0f, 3.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        followCameraDirection = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        followCameraDirection = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        std::cout << "Камера: " << camPos.x << ", " << camPos.y << ", "
                  << camPos.z << '\n';
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
        "Week 9 - Camera / LookAt",
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

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

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
    const glm::mat4 model(1.0f);
    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        float(WIDTH) / HEIGHT,
        0.1f,
        100.0f
    );

    float lastFrame = float(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        const float now = float(glfwGetTime());
        const float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        glClearColor(.075f, .09f, .14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::vec3 target = followCameraDirection
            ? camPos + camFront
            : glm::vec3(0.0f);
        const glm::mat4 view = glm::lookAt(camPos, target, camUp);

        glUseProgram(program);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}

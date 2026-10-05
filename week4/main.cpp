#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

float speed = 1.5f;

void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        speed += 2.0f * dt;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        speed -= 2.0f * dt;

    if (speed < 0.1f)
        speed = 0.1f;

    if (speed > 10.0f)
        speed = 10.0f;
}

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

uniform vec2 uOffset;
uniform float uScale;

void main()
{
    gl_Position = vec4(aPos.xy * uScale + uOffset, aPos.z, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Week 4", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

   if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
   	return -1;

    glfwSwapInterval(0);

    float vertices[] = {
         0.0f,  0.25f, 1.0f, 0.0f, 0.0f,
        -0.25f, -0.25f, 0.0f, 1.0f, 0.0f,
         0.25f, -0.25f, 0.0f, 0.0f, 1.0f
    };

    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shader = glCreateProgram();

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int locOffset = glGetUniformLocation(shader, "uOffset");
    int locScale = glGetUniformLocation(shader, "uScale");

    float lastFrame = (float)glfwGetTime();
    float angle = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        angle += speed * dt;

        float scale = 0.75f + 0.25f * std::sin(now * 2.0f);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        glUniform2f(
            locOffset,
            std::cos(angle) * 0.4f,
            std::sin(angle) * 0.4f
        );

        glUniform1f(locScale, scale);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader);

    glfwTerminate();

    return 0;
}

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

unsigned int makeShader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    int ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);

    if (!ok)
    {
        char info[512];
        glGetShaderInfoLog(id, 512, NULL, info);
        std::cout << "Shader error:\n" << info << std::endl;
    }

    return id;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Week 3", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
        return -1;

    float vertices[] = {
    	0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
    	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
    	0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    const char* vertexSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

void main()
{
    gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);
    vColor = aColor;
}
)";

    const char* fragmentSource = R"(
#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0 - vColor, 1.0);
}
)";

    unsigned int vertexShader = makeShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shader = glCreateProgram();

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    int ok = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, &ok);

    if (!ok)
    {
        char info[512];
        glGetProgramInfoLog(shader, 512, NULL, info);
        std::cout << "Link error:\n" << info << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const int STRIDE = 6 * sizeof(float);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        STRIDE, (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        STRIDE, (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
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

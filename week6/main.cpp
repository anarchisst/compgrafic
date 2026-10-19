
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>

const float PI = 3.14159265358979323846f;

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        900, 600, "Week 6 - Animation and Gravity", nullptr, nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    const char* vertexSource = R"(
#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 uOffset;

void main()
{
    gl_Position = vec4(aPos + uOffset, 0.0, 1.0);
}
)";

    const char* fragmentSource = R"(
#version 330 core

out vec4 FragColor;

uniform vec3 uColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}
)";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        2048 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float), nullptr
    );

    glEnableVertexAttribArray(0);

    GLint offsetLoc = glGetUniformLocation(program, "uOffset");
    GLint colorLoc = glGetUniformLocation(program, "uColor");

    const float squareSize = 0.10f;
    const float squareY = -0.98f;

    const float ballX = 0.55f;
    const float radius = 0.075f;
    const float groundY = -1.0f + radius;

    const float gravity = 2.8f;
    const float bounceVelocity = 2.0f;

    const float bouncePeriod = 2.0f * bounceVelocity / gravity;

    const float squareStart = -1.0f - squareSize;
    const float squareEnd = 1.0f;
    const float squareTravel = squareEnd - squareStart;

    const float squarePeriod = 3.0f * bouncePeriod;
    const float squareSpeed = squareTravel / squarePeriod;

    const float crossingCenter = ballX - squareSize * 0.5f;
    const float crossingTime = (crossingCenter - squareStart) / squareSpeed;

    const float bounceOffset =
        bouncePeriod * 0.5f - std::fmod(crossingTime, bouncePeriod);

    float squareVertices[] = {
        0.0f, 0.0f,
        squareSize, 0.0f,
        squareSize, squareSize,

        0.0f, 0.0f,
        squareSize, squareSize,
        0.0f, squareSize
    };

    const int segments = 64;
    float circleVertices[(segments + 2) * 2];

    circleVertices[0] = 0.0f;
    circleVertices[1] = 0.0f;

    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * i / segments;

        circleVertices[(i + 1) * 2] =
            std::cos(angle) * radius;

        circleVertices[(i + 1) * 2 + 1] =
            std::sin(angle) * radius;
    }

    double startTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float elapsed = static_cast<float>(
            glfwGetTime() - startTime
        );

        float squareTime = std::fmod(elapsed, squarePeriod);
        float squareX = squareStart + squareSpeed * squareTime;

        float ballTime = std::fmod(
            elapsed + bounceOffset,
            bouncePeriod
        );

        if (ballTime < 0.0f)
            ballTime += bouncePeriod;

        float ballHeight =
            bounceVelocity * ballTime -
            0.5f * gravity * ballTime * ballTime;

        float ballY = groundY + ballHeight;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClearColor(0.035f, 0.07f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            sizeof(squareVertices),
            squareVertices
        );

        glUniform2f(offsetLoc, squareX, squareY);
        glUniform3f(colorLoc, 0.15f, 0.9f, 0.55f);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            sizeof(circleVertices),
            circleVertices
        );

        glUniform2f(offsetLoc, ballX, ballY);
        glUniform3f(colorLoc, 1.0f, 0.35f, 0.55f);

        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

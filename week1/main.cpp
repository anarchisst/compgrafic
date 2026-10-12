#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

bool whiteBackground = false;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    whiteBackground = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

void onResize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Week 1", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(0);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    double lastTime = glfwGetTime();
    int frames = 0;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        float t = (float)glfwGetTime();

        if (whiteBackground)
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            float r = (std::sin(t * 2.0f) + 1.0f) * 0.5f * 0.3f;
            float g = (std::sin(t * 1.3f) + 1.0f) * 0.5f * 0.3f;

            glClearColor(r, g, 0.35f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frames++;

        double now = glfwGetTime();

        if (now - lastTime >= 1.0)
        {
            std::cout << "FPS: " << frames / (now - lastTime) << "\n";
            frames = 0;
            lastTime = now;
        }
    }

    glfwTerminate();
    return 0;
}

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

constexpr float Width = 1000;
constexpr float Height = 700;

struct Object {
    glm::vec2 position;
    bool alive = true;
};

glm::vec2 player;
std::vector<Object> bullets;
std::vector<Object> enemies;

int score = 0;
int lives = 3;
bool gameOver = false;

float fireTimer = 0;
float spawnTimer = 0;
float shield = 0;

std::mt19937 randomEngine(std::random_device{}());

GLuint program = 0;
GLuint vao = 0;
GLuint vbo = 0;
GLuint ebo = 0;

GLint offsetLocation;
GLint sizeLocation;
GLint colorLocation;

void resetGame() {
    player = {Width / 2, Height - 60};
    bullets.clear();
    enemies.clear();

    score = 0;
    lives = 3;
    gameOver = false;

    fireTimer = 0;
    spawnTimer = 0.8f;
    shield = 1;
}

bool hit(glm::vec2 a, glm::vec2 halfA, glm::vec2 b, glm::vec2 halfB) {
    glm::vec2 distance = glm::abs(a - b);

    return distance.x < halfA.x + halfB.x &&
           distance.y < halfA.y + halfB.y;
}

void updateGame(float dt, glm::vec2 direction, bool fire) {
    if (gameOver) {
        return;
    }

    dt = std::clamp(dt, 0.f, 0.05f);
    shield = std::max(0.f, shield - dt);

    if (glm::length(direction) > 1) {
        direction = glm::normalize(direction);
    }

    player += direction * 320.f * dt;
    player = glm::clamp(
        player,
        glm::vec2(25, 80),
        glm::vec2(Width - 25, Height - 25)
    );

    fireTimer -= dt;

    if (fire && fireTimer <= 0) {
        bullets.push_back({player + glm::vec2(0, -30)});
        fireTimer = 0.22f;
    }

    spawnTimer -= dt;

    if (spawnTimer <= 0) {
        float x = std::uniform_real_distribution<float>(
            30, Width - 30
        )(randomEngine);

        enemies.push_back({{x, 85}});
        spawnTimer = 0.8f;
    }

    for (auto& bullet : bullets) {
        bullet.position.y -= 600 * dt;

        if (bullet.position.y < 55) {
            bullet.alive = false;
        }
    }

    for (auto& enemy : enemies) {
        enemy.position.y += 110 * dt;

        if (enemy.position.y > Height + 25) {
            enemy.alive = false;
        }
    }

    for (auto& bullet : bullets) {
        for (auto& enemy : enemies) {
            if (!bullet.alive || !enemy.alive) {
                continue;
            }

            if (hit(bullet.position, {3, 9}, enemy.position, {22, 20})) {
                bullet.alive = false;
                enemy.alive = false;
                score += 100;
                break;
            }
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.alive &&
            hit(enemy.position, {21, 19}, player, {17, 20})) {
            enemy.alive = false;

            if (shield <= 0) {
                --lives;
                shield = 1.5f;

                if (lives == 0) {
                    gameOver = true;
                }
            }
        }
    }

    auto dead = [](const Object& object) {
        return !object.alive;
    };

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(), dead),
        bullets.end()
    );

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), dead),
        enemies.end()
    );
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char message[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, message);
        std::cerr << message << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool createGraphics() {
    const char* vertex = R"(
        #version 330 core
        layout(location = 0) in vec2 aPosition;
        uniform mat4 uProjection;
        uniform vec2 uOffset;
        uniform vec2 uSize;

        void main() {
            gl_Position = uProjection *
                vec4(aPosition * uSize + uOffset, 0.0, 1.0);
        }
    )";

    const char* fragment = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;

        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertex);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragment);

    if (!vs || !fs) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);

    if (!ok) {
        char message[1024];
        glGetProgramInfoLog(program, 1024, nullptr, message);
        std::cerr << message << '\n';
        return false;
    }

    const float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,

         0.0f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f
    };

    const unsigned indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices),
        vertices, GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float), nullptr
    );
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
        indices, GL_STATIC_DRAW
    );

    glUseProgram(program);

    auto projection = glm::ortho(
        0.f, Width, Height, 0.f, -1.f, 1.f
    );

    glUniformMatrix4fv(
        glGetUniformLocation(program, "uProjection"),
        1, GL_FALSE, glm::value_ptr(projection)
    );

    offsetLocation = glGetUniformLocation(program, "uOffset");
    sizeLocation = glGetUniformLocation(program, "uSize");
    colorLocation = glGetUniformLocation(program, "uColor");

    return true;
}

void draw(glm::vec2 position, glm::vec2 size,
          glm::vec3 color, bool triangle = false) {
    glUniform2fv(offsetLocation, 1, glm::value_ptr(position));
    glUniform2fv(sizeLocation, 1, glm::value_ptr(size));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));

    if (triangle) {
        glDrawArrays(GL_TRIANGLES, 4, 3);
    } else {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

const std::map<char, std::array<unsigned char, 7>> Font {
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'G', {14, 17, 16, 23, 17, 17, 15}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 21, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'V', {17, 17, 17, 17, 17, 10, 4}}
};

void text(const std::string& value, float x, float y, float size) {
    for (char character : value) {
        auto glyph = Font.find(character);

        if (glyph != Font.end()) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (glyph->second[row] & (1 << (4 - col))) {
                        draw(
                            {
                                x + (col + 0.5f) * size,
                                y + (row + 0.5f) * size
                            },
                            {size, size},
                            {1, 1, 1}
                        );
                    }
                }
            }
        }

        x += 6 * size;
    }
}

void renderGame(int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (width <= 0 || height <= 0) {
        return;
    }

    float scale = std::min(width / Width, height / Height);

    int w = static_cast<int>(Width * scale);
    int h = static_cast<int>(Height * scale);

    glViewport((width - w) / 2, (height - h) / 2, w, h);
    glUseProgram(program);
    glBindVertexArray(vao);

    text("SCORE " + std::to_string(score), 25, 20, 3);
    text("LIVES " + std::to_string(lives), 790, 20, 3);

    if (gameOver) {
        text("GAME OVER", 340, 280, 6);
        text("R TO RESTART", 356, 370, 4);
        return;
    }

    draw({Width / 2, 60}, {Width, 1}, {0.25f, 0.25f, 0.25f});

    glm::vec3 playerColor = shield > 0
        ? glm::vec3(0.2f, 0.8f, 1)
        : glm::vec3(1, 1, 1);

    draw(player, {44, 48}, playerColor, true);

    for (const auto& bullet : bullets) {
        draw(bullet.position, {4, 18}, {0.2f, 0.8f, 1});
    }

    for (const auto& enemy : enemies) {
        draw(enemy.position, {44, -40}, {1, 0.2f, 0.3f}, true);
    }
}

int main() {
    glfwSetErrorCallback([](int, const char* message) {
        std::cerr << message << '\n';
    });

    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(
        1000, 700,
        "Space Defender | WASD/Arrows: Move | Space: Fire | R: Restart | Esc: Exit",
        nullptr, nullptr
    );

    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(
        reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)
    )) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    int result = 0;

    if (createGraphics()) {
        resetGame();
        double previous = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            auto held = [&](int key) {
                return glfwGetKey(window, key) == GLFW_PRESS;
            };

            if (held(GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window, true);
            }

            if (gameOver && held(GLFW_KEY_R)) {
                resetGame();
            }

            glm::vec2 direction {
                int(held(GLFW_KEY_D) || held(GLFW_KEY_RIGHT)) -
                int(held(GLFW_KEY_A) || held(GLFW_KEY_LEFT)),

                int(held(GLFW_KEY_S) || held(GLFW_KEY_DOWN)) -
                int(held(GLFW_KEY_W) || held(GLFW_KEY_UP))
            };

            double now = glfwGetTime();

            if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
                updateGame(
                    static_cast<float>(now - previous),
                    direction,
                    held(GLFW_KEY_SPACE)
                );
            }

            previous = now;

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            renderGame(width, height);
            glfwSwapBuffers(window);

            if (width == 0 || height == 0) {
                glfwWaitEventsTimeout(0.05);
            }
        }
    } else {
        result = 1;
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}

// =====================================================================
//  КОМПЬЮТЕРЛІК ГРАФИКА — бір файлдық жоба
//
//  Бұл файл семестр бойы өседі. Әр аптада жаңа бөлік қосылады,
//  ескісі орнында қалады. Аптаның соңында:
//
//      git add . && git commit -m "week02" && git tag week02 && git push --tags
//
//  Қазіргі күйі: 2-АПТА — үшбұрыш (VBO + VAO)
//
//  Пернелер:
//      Пробел (басып тұр) — ақ фон            [1-апта, 3-тапсырма]
//      1                  — GL_TRIANGLES      (әдепкі)
//      2                  — GL_LINE_LOOP      [2-апта, 2-тапсырма]
//      W      (басып тұр) — wireframe         [2-апта, 3-тапсырма]
//      Esc                — шығу
// =====================================================================

#include <glad/gl.h>      // МІНДЕТТІ: glad әрқашан GLFW-дан БҰРЫН
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

// ---------------------------------------------------------------------
//  Баптаулар
// ---------------------------------------------------------------------
const int WIDTH  = 1280;   // [1-апта, 1-тапсырма] терезе 1280x720
const int HEIGHT = 720;

// ---------------------------------------------------------------------
//  Шейдерлер (әзірге ең қарапайым күйде, 3-аптада жақсартамыз)
// ---------------------------------------------------------------------
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
// [2-апта, 4-тапсырма] түс өзгертілді: қызғылт сары → жасыл (R, G, B, A)
void main() { FragColor = vec4(0.2, 0.9, 0.3, 1.0); }
)";

// ---------------------------------------------------------------------
//  Бағдарламаның күйі (пернелермен өзгереді)
// ---------------------------------------------------------------------
bool   whiteBackground = false;         // пробел басылып тұр ма
GLenum drawMode        = GL_TRIANGLES;  // қалай сызамыз: 1 немесе 2 пернесі
bool   wireframe       = false;         // W басылып тұр ма

// ---------------------------------------------------------------------
//  Терезе өлшемі өзгергенде шақырылады
// ---------------------------------------------------------------------
void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

// ---------------------------------------------------------------------
//  Пернетақтаны тексеру. Әр кадрда шақырылады.
// ---------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // [1-апта, 3-тапсырма] пробел басылып тұрғанда ғана фон ақ
    whiteBackground = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

    // [2-апта, 2-тапсырма] сызу режимін таңдау
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) drawMode = GL_TRIANGLES;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) drawMode = GL_LINE_LOOP;

    // [2-апта, 3-тапсырма] W басылып тұрғанда wireframe
    wireframe = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
}

// =====================================================================
//  MAIN
// =====================================================================
int main() {

    // -----------------------------------------------------------------
    //  1. GLFW-ны іске қосу
    // -----------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    // Қандай OpenGL нұсқасы керек екенін айтамыз.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // -----------------------------------------------------------------
    //  2. Терезе жасау
    // -----------------------------------------------------------------
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
                                          "Компьютерлік графика",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады. Видеокарта OpenGL 3.3-ті "
                     "қолдамауы мүмкін.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);              // осы терезенің контексі белсенді
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(0);                         // [1-апта, 4-тапсырма] VSync өшірулі

    // -----------------------------------------------------------------
    //  3. GLAD: OpenGL функцияларын жүктеу
    //     Контекст белсенді болғаннан КЕЙІН ғана. Ретін бұзсаң — бәрі құлайды.
    // -----------------------------------------------------------------
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";


    // === 2-АПТА: үшбұрыштың деректері мен буферлері ===

    // Вершина деректері — NDC координаттарында (-1 .. 1).
    // [2-апта, 1-тапсырма] 6 вершина = 2 үшбұрыш
    float vertices[] = {
        // 1-үшбұрыш (жоғары қараған)
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
        // 2-үшбұрыш (аударылған)
        -0.5f,  0.3f, 0.0f,
         0.5f,  0.3f, 0.0f,
         0.0f, -0.7f, 0.0f
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);                                  // VAO байлаймыз
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                      // VBO байлаймыз
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),          // деректі GPU-ға
                 vertices, GL_STATIC_DRAW);

    // GPU-ға байттарды қалай оқу керегін түсіндіреміз:
    // location=0, 3 float, нормаланбаған, қадам 3 float, ығысу 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // === Шейдерлерді компиляциялау (уақытша, 3-аптада жақсартамыз) ===
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
    //  4. Негізгі цикл
    // -----------------------------------------------------------------
    double lastFpsTime = glfwGetTime();   // [1-апта, 4-тапсырма] FPS есептеу
    int    frameCount  = 0;

    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        // [1-апта, 4-тапсырма] FPS-ті секундына бір рет қана шығарамыз
        frameCount++;
        double now = glfwGetTime();
        if (now - lastFpsTime >= 1.0) {
            std::cout << "FPS: " << frameCount << "\n";
            frameCount  = 0;
            lastFpsTime = now;
        }

        // --- Экранды тазалау ---
        if (whiteBackground) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);            // пробел → ақ
        } else {
            float t = (float)glfwGetTime();
            // [1-апта, 2-тапсырма] жиілік арттырылды: 0.5→2.0 және 0.3→1.3.
            // Соңындағы 0.3 — амплитуда, оны өзгертпедік.
            float r = (std::sin(t * 2.0f) + 1.0f) * 0.5f * 0.3f;
            float g = (std::sin(t * 1.3f) + 1.0f) * 0.5f * 0.3f;
            glClearColor(r, g, 0.35f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        // === 2-АПТА: сызу ===
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(drawMode, 0, 6);                        // 6 вершина

        glfwSwapBuffers(window);   // дайын кадрды экранға шығару
        glfwPollEvents();          // пернетақта/тінтуір оқиғаларын өңдеу
    }

    // -----------------------------------------------------------------
    //  5. Тазалау
    // -----------------------------------------------------------------
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

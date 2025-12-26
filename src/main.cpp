#include <SDL2/SDL.h>
#include <glad/gl.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h> // for file timestamps

// ===== SHADERS =====
const char* vertSrc = R"(
#version 330 core
layout (location = 0) in vec2 pos;
void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

// ===== HELPERS =====
std::string loadFile(const char* path) {
    std::ifstream in(path);
    if (!in) { 
        std::cerr << "Failed to open " << path << "\n"; 
        return ""; 
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetShaderInfoLog(s, 1024, nullptr, buf);
        std::cerr << "Shader compile error:\n" << buf << "\n";
    }
    return s;
}

GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, buf);
        std::cerr << "Program link error:\n" << buf << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ===== HOT-RELOAD FUNCTION =====
void reloadShader(GLuint &shaderProgram, const char* fragPath, const char* vertSrc) {
    std::string src = loadFile(fragPath);

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, src.c_str());
    GLint compiled;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        std::cerr << "Shader NOT compiled, keeping old program.\n";
        glDeleteShader(fs);
        return;
    }

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint newProg = linkProgram(vs, fs);

    if (newProg) {
        glDeleteProgram(shaderProgram);
        shaderProgram = newProg;
        //std::cout << "Shader reloaded successfully!\n";
    }
}

// ===== MAIN =====
int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* win = SDL_CreateWindow(
        "demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext ctx = SDL_GL_CreateContext(win);

    if (!gladLoaderLoadGL()) {
        puts("GLAD failed");
        return -1;
    }

    printf("GL %s\n", glGetString(GL_VERSION));

    // --- fullscreen quad ---
    float quad[] = {
        -1, -1,
         1, -1,
         1,  1,
        -1, -1,
         1,  1,
        -1,  1
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // --- fragment shader setup ---
    const char* fragPath = "shaders/effect.frag";
    GLuint shaderProgram = 0;

    // initial load
    {
        std::string src = loadFile(fragPath);
        GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, src.c_str());
        shaderProgram = linkProgram(vs, fs);
    }

    // --- main loop ---
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            // Hot-reload on key press (R)
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_R) {
                reloadShader(shaderProgram, fragPath, vertSrc);
            }
        }

        int w, h;
        SDL_GetWindowSize(win, &w, &h);

        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float ftime = SDL_GetTicks() * 0.001f;
        glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), ftime);
        glUniform2f(glGetUniformLocation(shaderProgram, "iResolution"), (float)w, (float)h);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(win);
    }

    SDL_Quit();
    return 0;
}


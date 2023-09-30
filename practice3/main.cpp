// 1850x1016
#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a < b ? b : a
#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>

std::string to_string(std::string_view str)
{
    return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message)
{
    throw std::runtime_error(to_string(message) + SDL_GetError());
}

void glew_fail(std::string_view message, GLenum error)
{
    throw std::runtime_error(to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
}

const char vertex_shader_source[] =
    R"(#version 330 core

uniform mat4 view;

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in float dist;

out vec4 color;
out float d;
void main()
{
    gl_Position = view * vec4(in_position, 0.0, 1.0);
    d = dist;
    color = in_color;
}
)";

const char fragment_shader_source[] =
    R"(#version 330 core

in vec4 color;
in float d;

uniform int be_dash;
uniform float t;

layout (location = 0) out vec4 out_color;

void main()
{
    if (be_dash == 0) {
        out_color = color;
    }
    else {
        if (mod(d + t * 100.f, 40.0) > 20.0)
            discard;
        out_color = color;
    }
}
)";

GLuint create_shader(GLenum type, const char *source)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Shader compilation failed: " + info_log);
    }
    return result;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint result = glCreateProgram();
    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragment_shader);
    glLinkProgram(result);

    GLint status;
    glGetProgramiv(result, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetProgramInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Program linkage failed: " + info_log);
    }

    return result;
}

struct vec2
{
    float x;
    float y;
};

struct vertex
{
    vec2 position;
    std::uint8_t color[4];
    float dist;
};

std::vector<vertex> lolkek;
std::vector<vertex> cheburek;

vec2 bezier(std::vector<vertex> const &vertices, float t)
{
    std::vector<vec2> points(vertices.size());

    for (std::size_t i = 0; i < vertices.size(); ++i)
        points[i] = vertices[i].position;

    // De Casteljau's algorithm
    for (std::size_t k = 0; k + 1 < vertices.size(); ++k)
    {
        for (std::size_t i = 0; i + k + 1 < vertices.size(); ++i)
        {
            points[i].x = points[i].x * (1.f - t) + points[i + 1].x * t;
            points[i].y = points[i].y * (1.f - t) + points[i + 1].y * t;
        }
    }
    return points[0];
}
void update_cheburek_inc(int cheburek_quality, GLuint cheburek_vbo)
{
    if (lolkek.size() > 1)
    {
        cheburek.clear();
        cheburek.reserve(cheburek_quality * lolkek.size() + 1);

        for (size_t i = 0; i < cheburek_quality * lolkek.size() + 1; ++i)
        {
            vec2 nv = bezier(lolkek, i * 1.f / lolkek.size() / cheburek_quality);
            float nd = i == 0 ? 0.f : std::hypotf(nv.x - cheburek.back().position.x, nv.y - cheburek.back().position.y) + cheburek.back().dist;
            cheburek.push_back(vertex{nv, {255, 0, 0, 255}, nd});
        }

        glBindBuffer(GL_ARRAY_BUFFER, cheburek_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * cheburek.size(), cheburek.data(), GL_STATIC_DRAW);
    }
}
void update_cheburek_dec(int cheburek_quality, GLuint cheburek_vbo)
{
    if (lolkek.size() > 1)
    {
        cheburek.clear();
        for (size_t i = 0; i < cheburek_quality * lolkek.size() + 1; ++i)
        {
            vec2 nv = bezier(lolkek, i * 1.f / lolkek.size() / cheburek_quality);
            float nd = i == 0 ? 0.f : std::hypotf(nv.x - cheburek.back().position.x, nv.y - cheburek.back().position.y) + cheburek.back().dist;
            cheburek.push_back(vertex{nv, {255, 0, 0, 255}, nd});
        }

        glBindBuffer(GL_ARRAY_BUFFER, cheburek_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * cheburek.size(), cheburek.data(), GL_STATIC_DRAW);
    }
    else
    {
        cheburek.clear();
        glBindBuffer(GL_ARRAY_BUFFER, cheburek_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, cheburek.data(), GL_STATIC_DRAW);
    }
}

int main()
try
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window *window = SDL_CreateWindow("Graphics course practice 3",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!window)
        sdl2_fail("SDL_CreateWindow: ");

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
        sdl2_fail("SDL_GL_CreateContext: ");

    SDL_GL_SetSwapInterval(0);

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    glClearColor(0.8f, 0.8f, 1.f, 0.f);

    auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    auto program = create_program(vertex_shader, fragment_shader);

    GLuint view_location = glGetUniformLocation(program, "view");
    GLuint be_dash_location = glGetUniformLocation(program, "be_dash");
    GLuint time_location = glGetUniformLocation(program, "t");

    GLuint lolkek_vao, lolkek_vbo;
    GLuint cheburek_vao, cheburek_vbo;

    glGenBuffers(1, &lolkek_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lolkek_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * lolkek.size(), lolkek.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &lolkek_vao);
    glBindVertexArray(lolkek_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 16, (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void *)8);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE, 16, (void *)12);

    glGenBuffers(1, &cheburek_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cheburek_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * cheburek.size(), cheburek.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &cheburek_vao);
    glBindVertexArray(cheburek_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 16, (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void *)8);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE, 16, (void *)12);

    int cheburek_quality = 4;

    auto last_frame_start = std::chrono::high_resolution_clock::now();
    glLineWidth(5.f);
    glPointSize(10);
    float time = 0.f;

    bool running = true;
    while (running)
    {
        for (SDL_Event event; SDL_PollEvent(&event);)
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    width = event.window.data1;
                    height = event.window.data2;
                    glViewport(0, 0, width, height);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int mouse_x = event.button.x;
                    int mouse_y = event.button.y;
                    lolkek.push_back(vertex{vec2{(float)(mouse_x), (float)(mouse_y)}, {0, 0, 0, 1}, 0});
                    glBindBuffer(GL_ARRAY_BUFFER, lolkek_vbo);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * lolkek.size(), lolkek.data(), GL_STATIC_DRAW);
                    update_cheburek_inc(cheburek_quality, cheburek_vbo);
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    if (lolkek.size()  < 1) {
                        continue;
                    }
                    glBindBuffer(GL_ARRAY_BUFFER, lolkek_vbo);
                    lolkek.pop_back();
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * lolkek.size(), lolkek.data(), GL_STATIC_DRAW);

                    update_cheburek_dec(cheburek_quality, cheburek_vbo);
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    cheburek_quality = MAX(1, cheburek_quality - 1);
                    update_cheburek_dec(cheburek_quality, cheburek_vbo);
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    cheburek_quality++;
                    update_cheburek_inc(cheburek_quality, cheburek_vbo);
                }
                break;
            }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        time += dt;
        glUniform1f(time_location, time);

        glClear(GL_COLOR_BUFFER_BIT);

        float view[16] =
            {
                2.f / (float)(width),
                0.f,
                0.f,
                -1.f,
                0.f,
                -2.f / (float)(height),
                0.f,
                1.f,
                0.f,
                0.f,
                1.f,
                0.f,
                0.f,
                0.f,
                0.f,
                1.f,
        };

        glUseProgram(program);

        glUniformMatrix4fv(view_location, 1, GL_TRUE, view);
        glUniform1i(be_dash_location, 0);
        
        glBindVertexArray(lolkek_vao);
        glDrawArrays(GL_LINE_STRIP, 0, lolkek.size());
        glDrawArrays(GL_POINTS, 0, lolkek.size());

        glUniform1i(be_dash_location, 1);

        glBindVertexArray(cheburek_vao);
        glDrawArrays(GL_LINE_STRIP, 0, cheburek.size());

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}
catch (std::exception const &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#define MIN(A, B) A < B ? A : B
#define MAX(A, B) A < B ? B : A

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>

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

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

out vec3 color;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    color = in_color;
}
)";

const char fragment_shader_source[] =
R"(#version 330 core

in vec3 color;

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(color, 1.0);
}
)";

GLuint create_shader(GLenum type, const char * source)
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

const size_t W = 20;
const size_t H = 20;
const size_t grid_vert_cnt = (W + 1) * (H + 1);
std::array<std::array<float, 3>, grid_vert_cnt> grid;
std::array<std::array<float, 3>, grid_vert_cnt> grid_values;

std::array<std::uint32_t, W * H * 6> grid_indices;

void initGrid() {
    for (size_t r = 0; r <= H; ++r) {
        for (size_t c = 0; c <= W; ++c) {
            auto x = 2.f * (float)c / (float)W - 1.f;
            auto y = 2.f * (float)r / (float)H - 1.f;
            grid[(H + 1) * r + c] = {x, y, 0.f};
        }
    }
}

void updateGridValues(float time) {
    for (size_t r = 0; r <= H; ++r) {
        for (size_t c = 0; c <= W; ++c) {
            float x = 2.f * (float)c / (float)W - 1.f;
            float y = 2.f * (float)r / (float)H - 1.f;
            float xi = cos(time);
            float yi = sin(time);
            float f = 0.2f * exp(-(pow(x - xi, 2) - pow(y - yi, 2) ) / (0.4f));
            grid_values[(H + 1) * r + c] = {MAX(f, 0), 0.f, MIN(f, 0)};
        }
    }
}

void initGridIndices() {
    for (size_t r = 0; r < H; ++r) {
        for (size_t c = 0; c <= W; ++c) {
            grid_indices[(r * (W) + c) * 6 + 0] = r * (W + 1) + c;
            grid_indices[(r * (W) + c) * 6 + 1] = r * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 2] = (r + 1) * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 3] = r * (W + 1) + c;
            grid_indices[(r * (W) + c) * 6 + 4] = (r + 1) * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 5] = (r + 1) * (W + 1) + c;
        }
    }
}

int main() try
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window * window = SDL_CreateWindow("Graphics course practice 4",
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

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    glClearColor(0.1f, 0.1f, 0.2f, 0.f);
    
    initGrid();
    initGridIndices();
    
    auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    auto program = create_program(vertex_shader, fragment_shader);

    GLuint grid_vao, grid_vbo, grid_ebo, values_vbo;
    
    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * grid.size(), grid.data(), GL_STATIC_DRAW);
    
    
    glGenBuffers(1, &grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * grid_indices.size(), grid_indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void *)0);
    
    glGenBuffers(1, &values_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, values_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * grid_values.size(), grid_values.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void *)0);

    auto last_frame_start = std::chrono::high_resolution_clock::now();

    float time = 0.f;

    std::map<SDL_Keycode, bool> button_down;

    bool running = true;
    while (running)
    {
        for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_WINDOWEVENT: switch (event.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                glViewport(0, 0, width, height);
                break;
            }
            break;
        case SDL_KEYDOWN:
            button_down[event.key.keysym.sym] = true;
            break;
        case SDL_KEYUP:
            button_down[event.key.keysym.sym] = false;
            break;
        }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        time += dt;

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(grid_vao);
        updateGridValues(time);
        glBindBuffer(GL_ARRAY_BUFFER, values_vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * grid_values.size(), grid_values.data(), GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, grid_indices.size(), GL_UNSIGNED_INT, (void*)0);
        
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}
catch (std::exception const & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

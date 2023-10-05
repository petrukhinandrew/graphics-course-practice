#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>

#include "utils.hpp"
#include "grid.hpp"
#include "isoline.hpp"

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

void rebindGrid(GLuint grid_vbo, GLuint grid_ebo)
{
    initGrid();
    initGridIndices();
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * GRID_VERTICES_NUM, grid.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * GRID_INDICES_NUM, grid_indices.data(), GL_STATIC_DRAW);
}

void increaseDetalization(GLuint grid_vbo, GLuint grid_ebo)
{
    if (W < 10)
    {
        W++;
    }
    else
    {
        W = MIN(W_LIM, W + 10);
    }
    if (H < 10)
    {
        H++;
    }
    else
    {
        H = MIN(H_LIM, H + 10);
    }

    recalculateDependentParameters();
    rebindGrid(grid_vbo, grid_ebo);
}

void decreaseDetalization(GLuint grid_vbo, GLuint grid_ebo)
{
    if (W <= 10)
    {
        W = MAX(1, W - 1);
    }
    else
    {
        W = MAX(10, W - 10);
    }
    if (H <= 10)
    {
        H = MAX(1, H - 1);
    }
    else
    {
        H = MAX(10, H - 10);
    }

    recalculateDependentParameters();
    rebindGrid(grid_vbo, grid_ebo);
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
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow("Graphics course practice 4",
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

    glClearColor(0.f, 0.f, 0.f, 1.f);

    initGrid();
    initGridIndices();
    updateGridValues(0.f);
    updateIsolines();

    auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    auto program = create_program(vertex_shader, fragment_shader);

    GLuint iso_loc = glGetUniformLocation(program, "iso");
    GLuint aspect_ratio_loc = glGetUniformLocation(program, "aspect_ratio");
    
    GLuint grid_vao, grid_vbo, grid_ebo, values_vbo;
    GLuint iso_vao, iso_vbo;
    ////////////////////////////////////
    glUseProgram(program);
    glUniform2f(aspect_ratio_loc, width < height ? 1.f : (float)height / (float)width, width < height ? (float)width / (float)height : 1.f);
    ////////////////////////////////////
    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * GRID_VERTICES_NUM, grid.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * GRID_INDICES_NUM, grid_indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), (void *)0);

    glGenBuffers(1, &values_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, values_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * GRID_VERTICES_NUM, grid_values.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_TRUE, sizeof(float), (void *)0);

    ///////////////////////////////

    glGenVertexArrays(1, &iso_vao);
    glBindVertexArray(iso_vao);

    glGenBuffers(1, &iso_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, iso_vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * ISOLINE_POINTS, isolines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), (void *)0);
    // //////////////////////////////////

    auto last_frame_start = std::chrono::high_resolution_clock::now();

    float time = 0.f;

    std::map<SDL_Keycode, bool> button_down;
    
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
                    glUniform2f(aspect_ratio_loc, width < height ? 1.f : (float)height / (float)width, width < height ? (float)width / (float)height : 1.f);
                    break;
                }
                break;
            case SDL_KEYDOWN:
                button_down[event.key.keysym.sym] = true;
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    decreaseDetalization(grid_vbo, grid_ebo);
                    break;
                case SDLK_RIGHT:
                    increaseDetalization(grid_vbo, grid_ebo);
                    break;
                case SDLK_UP:
                    increaseIsolineNumber();
                    break;
                case SDLK_DOWN:
                    decreaseIsolineNumber();
                    break;
                }
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
        glUniform1i(iso_loc, 0);
        glBindVertexArray(grid_vao);
        updateGridValues(time);

        glBindBuffer(GL_ARRAY_BUFFER, values_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * GRID_VERTICES_NUM, grid_values.data(), GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, GRID_INDICES_NUM, GL_UNSIGNED_INT, (void *)0);
        glUniform1i(iso_loc, 1);
        glBindVertexArray(iso_vao);
        updateIsolines();
        glBindBuffer(GL_ARRAY_BUFFER, iso_vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * ISOLINE_POINTS, isolines.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_LINES, 0, ISOLINE_POINTS);

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

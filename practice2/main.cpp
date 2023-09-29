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

const float a = 0.5;
const float PI = 3.1415926535897932384626433832795;
const vec2 VERTICES[8] = vec2[8](
    vec2(0.0, 0.0),
    vec2(a * cos(PI / 3.0), a * sin(PI / 3.0)),
    vec2(- a * cos(PI / 3.0), a * sin(PI / 3.0)),
    vec2(-a, 0.0),
    vec2(- a * cos(PI / 3.0), - a * sin(PI / 3.0)),
    vec2(a * cos(PI / 3.0), - a * sin(PI / 3.0)),
    vec2(a, 0.0),
    vec2(a * cos(PI / 3.0), a * sin(PI / 3.0))
);

const vec3 COLORS[3] = vec3[3](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

uniform mat4 transform;
out vec3 color;

void main()
{
    vec2 position = VERTICES[gl_VertexID];
    gl_Position = transform * vec4(position, 0.0, 1.0);
    color = COLORS[gl_VertexID];
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

int main() try
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_Window * window = SDL_CreateWindow("Graphics course practice 2",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!window)
        sdl2_fail("SDL_CreateWindow: ");

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
        sdl2_fail("SDL_GL_CreateContext: ");

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    glClearColor(0.8f, 0.8f, 1.f, 0.f);

    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

    GLuint program = create_program(vertex_shader, fragment_shader);

    GLuint vao;
    glUseProgram(program);
    
    GLint transform_uniform = glGetUniformLocation(program, "transform");
    glUniform1f(transform_uniform, 0.5);
    
    float time = 0.f;
    float scale = 0.5;
    float aspect_ratio;

    float x = 0;
    float dx = 0;
    
    float y = 0;
    float dy = 0;

    float transform[16] = {
        cos(time) * scale, sin(time) * scale, 0.0, 0.0, 
        -sin(time) * scale, cos(time) * scale, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 1.0};
    glGenVertexArrays(1, &vao);

    auto last_frame_start = std::chrono::high_resolution_clock::now();

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
                aspect_ratio = (float)width / (float)height;
                glViewport(0, 0, width, height);
                break;
            }
            break; 
        case SDL_KEYUP: 
        case SDL_KEYDOWN: switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_UP: 
                if (event.key.state == SDL_PRESSED)
                    dy = 1;
                else 
                    dy = 0;
            break;
            case SDL_SCANCODE_DOWN: 
                if (event.key.state == SDL_PRESSED)
                    dy = -1; 
                else 
                    dy = 0;
            break;
            case SDL_SCANCODE_LEFT: 
                if (event.key.state == SDL_PRESSED)
                    dx = -1; 
                else 
                    dx = 0;
            break;
            case SDL_SCANCODE_RIGHT: 
                if (event.key.state == SDL_PRESSED)
                    dx = 1;
                else 
                    dx = 0;
            break;
            default: break;
        }
        }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        time += dt;        
        
        x += dx * dt / aspect_ratio;
        y += dy * dt;
        
        transform[0] = cos(time) * scale / aspect_ratio; 
        transform[1] = - sin(time) * scale / aspect_ratio;
        
        transform[3] = x;
        
        transform[4] = sin(time) * scale;
        transform[5] = cos(time) * scale;

        transform[7] = y;
        
        last_frame_start = now;

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(transform_uniform, 1, GL_TRUE, transform);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);

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

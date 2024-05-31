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

GLuint  create_shader(GLenum shader_type, const char* shader_source) {
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);
    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE) { 
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string info_log (logLen, '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, info_log.data());
        throw std::runtime_error(info_log);
    }
    return shader;
}
GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertex_shader);
    glAttachShader(prog, fragment_shader);
    glLinkProgram(prog);
    GLint link_status;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        GLint logLen;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::string info_log (logLen, '\0');
        glGetProgramInfoLog(prog, logLen, nullptr, info_log.data());
        throw std::runtime_error(info_log);
    }
    return prog;
}
int main() try
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_Window * window = SDL_CreateWindow("Graphics course practice 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!window)
        sdl2_fail("SDL_CreateWindow: ");

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
    
    auto fragment_type = GL_FRAGMENT_SHADER;
    const char fragment_source[] = 
    R"(#version 330 core

    layout (location = 0) out vec4 out_color;
    // flat in vec3 color;
    
    void main() 
    {   
        bool v = int(gl_FragCoord.y) % 46 < 23;
        bool h = int(gl_FragCoord.x) % 80 < 40;
        vec3 c;
        if (h ^^ v)
            c = vec3(1.0, 1.0, 1.0);
        else 
            c = vec3(0.0, 0.0, 0.0);
        out_color = vec4(c, 1.0);
    }
    )";
    
    auto vertex_type = GL_VERTEX_SHADER;
    const char vertex_source[] = 
    R"(#version 330 core

    const vec2 VERTICES[3] = vec2[3](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0)
    );
    
    // flat out vec3 color;

    void main()
    {
        gl_Position = vec4(VERTICES[gl_VertexID], 0.0, 1.0);
        // color = vec3(gl_VertexID, gl_VertexID / 2, gl_VertexID);
    }
    )";

    GLuint vs = create_shader(vertex_type, vertex_source);
    GLuint fs = create_shader(fragment_type, fragment_source);
    GLuint p = create_program(vs, fs);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);

    bool running = true;
    while (running)
    {
        for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        }

        if (!running)
            break;

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(p);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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

#include <chrono>

#include "shaders.hpp"
#include "utils.hpp"
#include "grid.hpp"
#include "isoline.hpp"

template<size_t N>
void increaseDetalization(grid_g<N>& g)
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
    g.reload();
}

template<size_t N>
void decreaseDetalization(grid_g<N>& g)
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
    g.reload();
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
    grid_g<GRID_VERTICES_NUM_LIMIT> real_g;
    isoline_bro<GRID_VERTICES_NUM_LIMIT, 20> iso_bro(5);

    auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    auto program = create_program(vertex_shader, fragment_shader);

    GLuint iso_loc = glGetUniformLocation(program, "iso");
    GLuint aspect_ratio_loc = glGetUniformLocation(program, "aspect_ratio");
        
    glUseProgram(program);
    glUniform2f(aspect_ratio_loc, width < height ? 1.f : (float)height / (float)width, width < height ? (float)width / (float)height : 1.f);
    
    iso_bro.becomeBro();
    real_g.prepare();

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
                    decreaseDetalization(real_g);
                    break;
                case SDLK_RIGHT:
                    increaseDetalization(real_g);
                    break;
                case SDLK_UP:
                    iso_bro.workout();
                    break;
                case SDLK_DOWN:
                    iso_bro.weaken();
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
        real_g.shot(time);
        
        glUniform1i(iso_loc, 1);
        iso_bro.doThingsBro(real_g.showEm());

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

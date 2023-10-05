#pragma once
#include <iostream>
#include <array>

#define MIN(A, B) A < B ? A : B
#define MAX(A, B) A < B ? B : A

using std::size_t;

struct parameters {

};

const size_t W_LIM = 70;
const size_t H_LIM = 70;

const size_t GRID_VERTICES_NUM_LIMIT = (W_LIM + 1) * (H_LIM + 1);
const size_t GRID_INDICES_NUM_LIMIT = W_LIM * H_LIM * 6;
const size_t ISOLINES_ALLOC = GRID_VERTICES_NUM_LIMIT * GRID_VERTICES_NUM_LIMIT * 2;
size_t W = 25;
size_t H = 25;
size_t GRID_VERTICES_NUM = (W + 1) * (H + 1);
size_t GRID_INDICES_NUM = W * H * 6;

float DX = 2.f / (float)W;
float DY = 2.f / (float)H;

std::array<std::array<float, 2>, GRID_VERTICES_NUM_LIMIT> grid;
std::array<float, GRID_VERTICES_NUM_LIMIT> grid_values;
std::array<std::uint32_t, GRID_INDICES_NUM_LIMIT> grid_indices;
std::array<std::array<float, 2>, ISOLINES_ALLOC> isolines;

int ISOLINE_POINTS = 0;
int ISOLINE_NUM = 1;
const size_t XI_NUM = 7;
const int F_MIN = -4;
const int F_MAX = 6;

void recalculateDependentParameters()
{
    GRID_VERTICES_NUM = (W + 1) * (H + 1);
    GRID_INDICES_NUM = W * H * 6;
    DX = 2.f / (float)W;
    DY = 2.f / (float)H;
}

const char vertex_shader_source[] =
    R"(#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in float in_value;

out vec4 color;
uniform vec2 aspect_ratio;
uniform int iso;

void main()
{
    gl_Position = vec4(in_position * aspect_ratio, 0.0, 1.0);
    if (iso == 0) {
        float f = in_value > 0 ? 1 : -1; 
        float grey = 1 - abs(in_value);
        color = vec4(f > grey ? f : grey, grey, -f > grey ? -f : grey, 1.0);
    } else {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
)";

const char fragment_shader_source[] =
    R"(#version 330 core

in vec4 color;

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = color;
}
)";

struct point {
    float x, y;
    
    static point by(int r, int c) {
        return new_point(r, c);
    }
};

point new_point(int r, int c)
{
    return point{2.f * (float)c / (float)W - 1.f, 2.f * (float)r / (float)H - 1.f};
}

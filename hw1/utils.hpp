#pragma once
#include <iostream>
#include <array>

#define MIN(A, B) A < B ? A : B
#define MAX(A, B) A < B ? B : A

const size_t W_LIM = 80;
const size_t H_LIM = 80;

const size_t GRID_VERTICES_NUM_LIMIT = (W_LIM + 1) * (H_LIM + 1);
const size_t GRID_INDICES_NUM_LIMIT = W_LIM * H_LIM * 6;
size_t W = 25;
size_t H = 25;
size_t GRID_VERTICES_NUM = (W + 1) * (H + 1);
size_t GRID_INDICES_NUM = W * H * 6;

float DX = 2.f / (float)W;
float DY = 2.f / (float)H;

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

struct point {
    float x, y;
    
    bool operator<(const point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }

    bool operator==(const point& other) const {
        return x == other.x && y == other.y;
    }
};

point new_point(int r, int c)
{
    return point{2.f * (float)c / (float)W - 1.f, 2.f * (float)r / (float)H - 1.f};
}

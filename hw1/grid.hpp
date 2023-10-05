#pragma once
#include "utils.hpp"

struct grid
{
};

void initGrid()
{
    for (size_t r = 0; r <= H; ++r)
    {
        for (size_t c = 0; c <= W; ++c)
        {
            auto x = 2.f * (float)c / (float)W - 1.f;
            auto y = 2.f * (float)r / (float)H - 1.f;
            grid[(W + 1) * r + c] = {x, y};
        }
    }
}

void initGridIndices()
{
    for (size_t r = 0; r < H; ++r)
    {
        for (size_t c = 0; c <= W; ++c)
        {
            grid_indices[(r * (W) + c) * 6 + 0] = r * (W + 1) + c;
            grid_indices[(r * (W) + c) * 6 + 1] = r * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 2] = (r + 1) * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 3] = r * (W + 1) + c;
            grid_indices[(r * (W) + c) * 6 + 4] = (r + 1) * (W + 1) + c + 1;
            grid_indices[(r * (W) + c) * 6 + 5] = (r + 1) * (W + 1) + c;
        }
    }
}

void updateGridValues(float time)
{
    for (size_t r = 0; r <= H; ++r)
    {
        for (size_t c = 0; c <= W; ++c)
        {
            float x = 2.f * (float)c / (float)W - 1.f;
            float y = 2.f * (float)r / (float)H - 1.f;
            float f = 0;
            for (int i = 0; i < XI_NUM; ++i)
            {
                float xi = cos(time * pow(-1, i) * (2.f * (float)i / (float)XI_NUM - 1.f));
                float yi = sin(time * pow(-1, i) * (2.f * (float)i / (float)XI_NUM - 1.f));
                float ri = i * 0.05f;
                float ci = (float)i * pow(-1, i);
                float numer = pow(x - xi, 2) + pow(y - yi, 2);
                f += ci / exp(numer / pow(ri, 2));
            }
            grid_values[(W + 1) * r + c] = f;
        }
    }
}

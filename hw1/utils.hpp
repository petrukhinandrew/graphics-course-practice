#pragma once
#include <iostream>
#include <array>

#define MIN(A, B) A < B ? A : B
#define MAX(A, B) A < B ? B : A

struct big_boss
{
    size_t W = 25;
    size_t H = 25;
    static constexpr size_t W_LIM = 70;
    static constexpr size_t H_LIM = 70;

    size_t GRID_VERTICES_NUM = (W + 1) * (H + 1);
    size_t GRID_INDICES_NUM = W * H * 6;
    static constexpr size_t GRID_VERTICES_NUM_LIMIT = (W_LIM + 1) * (H_LIM + 1);
    static constexpr size_t GRID_INDICES_NUM_LIMIT = GRID_VERTICES_NUM_LIMIT * 6;
    
    float DX = 2.f / (float)W;
    float DY = 2.f / (float)H;

    void doBusiness()
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
        std::cout << W << std::endl;
        _rule();
    }

    void undoBusiness()
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

        _rule();
    }

private:
    void _rule()
    {
        GRID_VERTICES_NUM = (W + 1) * (H + 1);
        GRID_INDICES_NUM = W * H * 6;
        DX = 2.f / (float)W;
        DY = 2.f / (float)H;
    }
};

struct point
{
    float x, y;

    bool operator<(const point &other) const
    {
        return x < other.x || (x == other.x && y < other.y);
    }

    bool operator==(const point &other) const
    {
        return x == other.x && y == other.y;
    }
};

point new_point(int r, int c, float W, float H)
{
    return point{2.f * (float)c / W - 1.f, 2.f * (float)r / H - 1.f};
}

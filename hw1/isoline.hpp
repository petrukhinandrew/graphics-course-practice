#pragma once

#include <map>
#include <array>
#include "utils.hpp"

template <int MUSCLE_LIMIT, int SIZE_LIMIT>
class isoline_bro
{
public:
    GLuint vbo, ebo;
    isoline_bro(int init_size): _size(init_size), _next_idx(0), _next_point(0) {

    } 
    
    void bro() {
        _muscleBro();
        _bindBro();
    }
    
    void workout()
    {
        _size = MIN(_size + 1, SIZE_LIMIT);
    }
    
    void weaken()
    {
        _size = MAX(_size - 1, 0);
    }

private:
    int _size;
    std::array<std::array<float, 2>, MUSCLE_LIMIT> _points;
    size_t _next_point, _next_idx;
    std::map<point, int> _mapping;
    std::array<std::uint32_t, MUSCLE_LIMIT * SIZE_LIMIT> _indices;
    
    void _bindBro() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * _next_point, _points.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * _next_idx, _indices.data(), GL_STATIC_DRAW);
    }
    
    void _muscleBro()
    {
        this->_next_point_idx = 0;

        for (size_t r = 0; r < H; ++r)
        {
            for (size_t c = 0; c < W; ++c)
            {
                std::array<float, 4> vs;
                int vsi = 0;
                for (auto i : {r * (W + 1) + c, r * (W + 1) + c + 1, (r + 1) * (W + 1) + c + 1, (r + 1) * (W + 1) + c})
                {
                    vs[vsi] = grid_values[i];
                    vsi++;
                }
                point v1 = new_point(r, c);
                point v2 = new_point(r, c + 1);
                point v3 = new_point(r + 1, c + 1);
                point v4 = new_point(r + 1, c);

                for (int cur_isoline_idx = 0; cur_isoline_idx < this->_size; ++cur_isoline_idx)
                {
                    float k = (float)F_MIN + (F_MAX - F_MIN) * (float)cur_isoline_idx / (float)this->_size;
                    if (abs(k) < 0.0001)
                        continue;
                    int lolkek = (vs[0] > k) + 2 * (vs[1] > k) + 4 * (vs[2] > k) + 8 * (vs[3] > k);
                    this->_squareBro((float)k, lolkek, vs, v1, v2, v3, v4);
                }
            }
        }
    }

    void _squareBro(float l, int r, std::array<float, 4> &vals, point p1, point p2, point p3, point p4)
    {
        point n{(p3.x + p4.x) / 2.f, p4.y};
        point w{p1.x, (p1.y + p4.y) / 2.f};
        point e{p2.x, (p2.y + p3.y) / 2.f};
        point s{(p1.x + p2.x) / 2.f, p1.y};

        if (r == 1 || r == 14)
        {
            point a = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
            point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
            this->_lineBro(a, b);
            return;
        }
        if (r == 2 || r == 13)
        {
            point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
            point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
            this->_lineBro(a, b);
            return;
        }
        if (r == 3 || r == 12)
        {
            point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
            point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[3] - vals[0])};
            this->_lineBro(a, b);
            return;
        }

        if (r == 4 || r == 11)
        {
            point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
            point b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
            this->_lineBro(a, b);
            return;
        }

        if (r == 5)
        {
            point a = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
            point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
            this->_lineBro(a, b);
            a = {p2.x, p2.y + DY * abs(l - vals[1]) / (vals[1] + vals[2])};
            b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
            this->_lineBro(a, b);
            return;
        }

        if (r == 6 || r == 9)
        {
            point a = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
            point b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
            this->_lineBro(a, b);
            return;
        }

        if (r == 7 || r == 8)
        {
            point a = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
            point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
            this->_lineBro(a, b);
            return;
        }

        if (r == 10)
        {
            point a = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
            point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
            this->_lineBro(a, b);
            a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
            b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
            this->_lineBro(a, b);
            return;
        }
    }

    void _lineBro(point p1, point p2)
    {
        _pointBro(p1);
        _pointBro(p2);
    }

    void _pointBro(point p) {
        if (_mapping.find(p) != _mapping.end()) {
            _indices[_next_idx] = _mapping[p];
            ++_next_idx;
        } else {
            _points[_next_point] = {p.x, p.y};
            _mapping[p] = _next_point;
            ++_next_point;
        }
    }

};

void increaseIsolineNumber()
{
    ISOLINE_NUM = MIN(ISOLINE_NUM + 1, 20);
}

void decreaseIsolineNumber()
{
    ISOLINE_NUM = MAX(ISOLINE_NUM - 1, 0);
}

void insertIsoPoints(point p1, point p2)
{
    isolines[ISOLINE_POINTS] = {p1.x, p1.y};
    ISOLINE_POINTS++;
    isolines[ISOLINE_POINTS] = {p2.x, p2.y};
    ISOLINE_POINTS++;
}

void interpolateIsoPoints(float l, int r, std::array<float, 4> &vals, point p1, point p2, point p3, point p4)
{
    point n{(p3.x + p4.x) / 2.f, p4.y};
    point w{p1.x, (p1.y + p4.y) / 2.f};
    point e{p2.x, (p2.y + p3.y) / 2.f};
    point s{(p1.x + p2.x) / 2.f, p1.y};

    if (r == 1 || r == 14)
    {
        point a = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
        point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
        insertIsoPoints(a, b);
        return;
    }
    if (r == 2 || r == 13)
    {
        point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
        point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
        insertIsoPoints(a, b);
        return;
    }
    if (r == 3 || r == 12)
    {
        point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
        point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[3] - vals[0])};
        insertIsoPoints(a, b);
        return;
    }

    if (r == 4 || r == 11)
    {
        point a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
        point b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
        insertIsoPoints(a, b);
        return;
    }

    if (r == 5)
    {
        point a = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
        point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
        insertIsoPoints(a, b);
        a = {p2.x, p2.y + DY * abs(l - vals[1]) / (vals[1] + vals[2])};
        b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
        insertIsoPoints(a, b);
        return;
    }

    if (r == 6 || r == 9)
    {
        point a = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
        point b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
        insertIsoPoints(a, b);
        return;
    }

    if (r == 7 || r == 8)
    {
        point a = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
        point b = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
        insertIsoPoints(a, b);
        return;
    }

    if (r == 10)
    {
        point a = {p1.x, p1.y + DY * abs(l - vals[0]) / abs(vals[0] - vals[3])};
        point b = {p1.x + DX * abs(l - vals[0]) / abs(vals[0] - vals[1]), p1.y};
        insertIsoPoints(a, b);
        a = {p2.x, p2.y + DY * abs(l - vals[1]) / abs(vals[1] - vals[2])};
        b = {p4.x + DX * abs(l - vals[3]) / abs(vals[3] - vals[2]), p4.y};
        insertIsoPoints(a, b);
        return;
    }
}

void updateIsolines()
{
    ISOLINE_POINTS = 0;

    for (size_t r = 0; r < H; ++r)
    {
        for (size_t c = 0; c < W; ++c)
        {
            std::array<float, 4> vs;
            int vsi = 0;
            for (auto i : {r * (W + 1) + c, r * (W + 1) + c + 1, (r + 1) * (W + 1) + c + 1, (r + 1) * (W + 1) + c})
            {
                vs[vsi] = grid_values[i];
                vsi++;
            }
            point v1 = point::by(r, c);
            point v2 = new_point(r, c + 1);
            point v3 = new_point(r + 1, c + 1);
            point v4 = new_point(r + 1, c);
            for (int ison = 0; ison < ISOLINE_NUM; ++ison)
            {
                float k = (float)F_MIN + (F_MAX - F_MIN) * (float)ison / (float)ISOLINE_NUM;
                if (abs(k) < 0.0001)
                    continue;
                int lolkek = (vs[0] > k) + 2 * (vs[1] > k) + 4 * (vs[2] > k) + 8 * (vs[3] > k);
                interpolateIsoPoints((float)k, lolkek, vs, v1, v2, v3, v4);
            }
        }
    }
}
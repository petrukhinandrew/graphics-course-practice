#pragma once

#include <map>
#include <array>
#include "utils.hpp"

template <int MUSCLE_LIMIT, int SIZE_LIMIT>
class isoline_bro
{
public:
    isoline_bro(int init_size, big_boss& boss) : _boss(boss), _size(init_size), _next_idx(0), _next_point(0)
    {
    }

    void becomeBro()
    {
        _initBro();
    }

    void doThingsBro(float *bro_data)
    {
        _muscleBro(bro_data);
        _bindBro();
        _poseBro();
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
    GLuint vao, vbo, ebo;
    int _size;
    big_boss& _boss;
    std::array<std::array<float, 2>, MUSCLE_LIMIT> _points;
    size_t _next_point, _next_idx;
    std::map<point, std::uint32_t> _mapping;
    std::array<std::uint32_t, MUSCLE_LIMIT * SIZE_LIMIT> _indices;

    static constexpr int F_MIN = -4;
    static constexpr int F_MAX = 6;

    void _initBro()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        _bindBro();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), (void *)0);
    }

    void _bindBro()
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * _next_idx, _indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * _next_point, _points.data(), GL_STATIC_DRAW);
    }

    void _poseBro()
    {
        glDrawElements(GL_LINES, _next_idx, GL_UNSIGNED_INT, (void *)0);
    }

    void _muscleBro(float *bro_data)
    {
        _next_point = 0;
        _next_idx = 0;
        _mapping.clear();

        for (size_t r = 0; r < _boss.H; ++r)
        {
            for (size_t c = 0; c < _boss.W; ++c)
            {
                std::array<float, 4> vs;
                int vsi = 0;
                for (auto i : {r * (_boss.W + 1) + c, r * (_boss.W + 1) + c + 1, (r + 1) * (_boss.W + 1) + c + 1, (r + 1) * (_boss.W + 1) + c})
                {
                    vs[vsi] = bro_data[i];
                    vsi++;
                }
                float W = (float)_boss.W;
                float H = (float)_boss.H;
                point v1 = new_point(r, c, W, H);
                point v2 = new_point(r, c + 1, W, H);
                point v3 = new_point(r + 1, c + 1, W, H);
                point v4 = new_point(r + 1, c, W, H);

                for (int cruBro = 0; cruBro < _size; ++cruBro)
                {
                    float k = (float)F_MIN + (F_MAX - F_MIN) * (float)cruBro / (float)this->_size;
                    if (abs(k) < 0.0001)
                        continue;
                    int kekBro = (vs[0] > k) + 2 * (vs[1] > k) + 4 * (vs[2] > k) + 8 * (vs[3] > k);
                    this->_squareBro((float)k, kekBro, vs, v1, v2, v3, v4);
                }
            }
        }
    }

    void _squareBro(float l, int r, std::array<float, 4> &vals, point p1, point p2, point p3, point p4)
    {
        float DX = _boss.DX;
        float DY = _boss.DY;

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

    void _pointBro(point p)
    {
        if (_mapping.find(p) == _mapping.end())
        {
            _points[_next_point] = {p.x, p.y};
            _mapping[p] = _next_point;
            _next_point++;
        }
        _indices[_next_idx] = _mapping[p];
        _next_idx++;
    }
};
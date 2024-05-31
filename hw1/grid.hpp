#pragma once
#include "utils.hpp"
#include <map>

class grid_g
{
public:
    grid_g(big_boss& boss): _boss(boss)
    {
    }

    void prepare()
    {
        _becomeTrueG();
        _initVertices();
        _initIndices();
        _bindEmAll();
    }

    void reload()
    {
        _initVertices();
        _initIndices();
        glBindVertexArray(vao);
        _crushEm();
        _paintEm();
    }

    float *showEm()
    {
        return _values.data();
    }

    void shot(float time)
    {
        _explainEmAll(time);
        _paintItBlack();
    }

private:
    void _paintItBlack()
    {
        glBindVertexArray(vao);
        _paintEm();
        glDrawElements(GL_TRIANGLES, _boss.GRID_INDICES_NUM, GL_UNSIGNED_INT, (void *)0);
    }

    void _crushEm()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * _boss.GRID_INDICES_NUM, _indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * _boss.GRID_VERTICES_NUM, _vertices.data(), GL_STATIC_DRAW);
    }

    void _paintEm()
    {
        glBindBuffer(GL_ARRAY_BUFFER, values_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _boss.GRID_VERTICES_NUM, _values.data(), GL_STATIC_DRAW);
    }

    void _becomeTrueG()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vertices_vbo);
        glGenBuffers(1, &values_vbo);
        glGenBuffers(1, &ebo);
    }

    void _bindEmAll()
    {
        glBindVertexArray(vao);

        _crushEm();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), (void *)0);

        _paintEm();

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_TRUE, sizeof(float), (void *)0);
    }

    void _initVertices()
    {
        for (size_t r = 0; r <= _boss.H; ++r)
        {
            for (size_t c = 0; c <= _boss.W; ++c)
            {
                auto x = 2.f * (float)c / (float)_boss.W - 1.f;
                auto y = 2.f * (float)r / (float)_boss.H - 1.f;
                _vertices[r * (_boss.W + 1) + c] = {x, y};
            }
        }
    }

    void _initIndices()
    {
        for (size_t r = 0; r < _boss.H; ++r)
        {
            for (size_t c = 0; c <= _boss.W; ++c)
            {
                _indices[(r * (_boss.W) + c) * 6 + 0] = r * (_boss.W + 1) + c;
                _indices[(r * (_boss.W) + c) * 6 + 1] = r * (_boss.W + 1) + c + 1;
                _indices[(r * (_boss.W) + c) * 6 + 2] = (r + 1) * (_boss.W + 1) + c + 1;
                _indices[(r * (_boss.W) + c) * 6 + 3] = r * (_boss.W + 1) + c;
                _indices[(r * (_boss.W) + c) * 6 + 4] = (r + 1) * (_boss.W + 1) + c + 1;
                _indices[(r * (_boss.W) + c) * 6 + 5] = (r + 1) * (_boss.W + 1) + c;
            }
        }
    }

    void _explainEmAll(float time)
    {
        for (size_t r = 0; r <= _boss.H; ++r)
        {
            for (size_t c = 0; c <= _boss.W; ++c)
            {
                float x = 2.f * (float)c / (float)_boss.W - 1.f;
                float y = 2.f * (float)r / (float)_boss.H - 1.f;
                _values[(_boss.W + 1) * r + c] = _explainThisPoint(x, y, time);
            }
        }
    }

    float _explainThisPoint(float x, float y, float time)
    {
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
        return f;
    }

    big_boss& _boss;
    GLuint vao, values_vbo, vertices_vbo, ebo;
    std::array<std::array<float, 2>, big_boss::GRID_VERTICES_NUM_LIMIT> _vertices;
    std::array<float, big_boss::GRID_VERTICES_NUM_LIMIT> _values;
    std::array<std::uint32_t, big_boss::GRID_INDICES_NUM_LIMIT> _indices;
    
    static constexpr size_t XI_NUM = 7;
};

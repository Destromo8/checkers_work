#pragma once
#include <stdlib.h>
#include <stdint.h>

typedef int8_t POS_T;

// Структура, описывающая ход шашки
struct move_pos
{
    POS_T x, y;             // начальная позиция (откуда)
    POS_T x2, y2;           // конечная позиция (куда)
    POS_T xb = -1, yb = -1; // позиция битой шашки (если есть)

    // Конструктор хода без взятия
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2)
        : x(x), y(y), x2(x2), y2(y2) {}

    // Конструктор хода со взятием шашки соперника
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb) {}

    // Переопределение оператора == (сравнение ходов)
    bool operator==(const move_pos &other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }

    // Переопределение оператора != (обратное сравнение)
    bool operator!=(const move_pos &other) const
    {
        return !(*this == other);
    }
};

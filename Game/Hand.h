#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс обработки ввода игрока
class Hand
{
public:
    Hand(Board *board) : board(board) {}

    // Получить выбранную ячейку игроком
    std::tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;
        int xc = -1, yc = -1;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent)) // если произошло событие
            {
                switch (windowEvent.type) // обработка типа события
                {
                case SDL_QUIT: // выход из игры
                    resp = Response::QUIT;
                    break;

                case SDL_MOUSEBUTTONDOWN: // клик мышью по полю
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);

                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1) // клик по кнопке "назад"
                    {
                        resp = Response::BACK;
                    }
                    else if (xc == -1 && yc == 8) // клик по кнопке "повторить"
                    {
                        resp = Response::REPLAY;
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8) // клик по ячейке доски
                    {
                        resp = Response::CELL;
                    }
                    else
                    {
                        xc = -1;
                        yc = -1;
                    }
                    break;

                case SDL_WINDOWEVENT: // изменение размера окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size();
                        break;
                    }
                }
                if (resp != Response::OK)
                    break;
            }
        }
        return {resp, xc, yc}; // возвращает тип действия и координаты клетки
    }

    // Ожидание действий игрока (например, "повторить" или "выход")
    Response wait() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT: // выход
                    resp = Response::QUIT;
                    break;

                case SDL_WINDOWEVENT_SIZE_CHANGED: // изменение размера окна
                    board->reset_window_size();
                    break;

                case SDL_MOUSEBUTTONDOWN: { // клик мышью
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8) // клик по кнопке "повторить"
                        resp = Response::REPLAY;
                }
                break;
                }
                if (resp != Response::OK)
                    break;
            }
        }
        return resp; // возвращает тип действия
    }

private:
    Board *board;
};

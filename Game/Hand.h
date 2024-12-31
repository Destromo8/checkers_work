#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// methods for hands
class Hand
{
  public:
    Hand(Board *board) : board(board)
    {
    }
    tuple<Response, POS_T, POS_T> get_cell() const // массив выбранных ячеек
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;
        int xc = -1, yc = -1;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent)) // если какое то событие произошло
            {
                switch (windowEvent.type) // выбор события
                {
                case SDL_QUIT: // событие выхода: ответ = выход
                    resp = Response::QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN: // перемещение мышки по полю
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1) // выбор действия на экране
                    {
                        resp = Response::BACK; // кнопка назад
                    }
                    else if (xc == -1 && yc == 8) // кнопка повторить
                    {
                        resp = Response::REPLAY;
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8) //кнопка ячейки
                    {
                        resp = Response::CELL;
                    }
                    else
                    {
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT: // восстановление размера экрана
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
        return {resp, xc, yc}; // возвращает выбор пользователя, позиция мышки X и Y
    }

    Response wait() const // класс ожидания выбора пользователем действия
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true) // бесконченый цикл ожидания выбора
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type) // выбор действия
                {
                case SDL_QUIT: // кнопка выхода
                    resp = Response::QUIT;
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED: // восстановление размера экрана
                    board->reset_window_size();
                    break;
                case SDL_MOUSEBUTTONDOWN: { // положение мышки на экране
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8) // повторить игру
                        resp = Response::REPLAY;
                }
                break;
                }
                if (resp != Response::OK)
                    break;
            }
        }
        return resp; // возврат выбора 
    }

  private:
    Board *board;
};

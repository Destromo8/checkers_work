#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // to start checkers
    int play()
    {
        auto start = chrono::steady_clock::now(); // переменная для хранении продолжительности игры (времени партии)
        // Проверка на повтор игры (если игра сыграна и пользователь хочет ещё одну игру)
        if (is_replay)
        {
            //Определеяется заново логика игры, перезапускается конфигурация с настройками и перерисовывается экран игры (для новой игры)
            logic = Logic(&board, &config);
            config.reload(); // загружаются настройки из settings.json
            board.redraw();
        }
        else
        {
            board.start_draw(); // рисуется начальное окно
        }
        is_replay = false;

        int turn_num = -1;  // номер хода
        bool is_quit = false; // выйти из игры?
        const int Max_turns = config("Game", "MaxNumTurns"); // берем из настроек максимальное число ходов (по дефолту 120)
        
        // цикл игры по ходам: 
        while (++turn_num < Max_turns) // в начале увеличиваем на 1, чтобы выбрать чей ход
        {
            beat_series = 0;
            logic.find_turns(turn_num % 2); // определяем чей ход: белых или черных
            if (logic.turns.empty())
                break;
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));
            // Ход игрока
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot")))
            {
                auto resp = player_turn(turn_num % 2); // ход игрока
                if (resp == Response::QUIT) // выбор игроком кнопки выйти
                {
                    is_quit = true; //выход = Истина
                    break;
                }
                else if (resp == Response::REPLAY) // кнопка играть ещё раз
                {
                    is_replay = true;
                    break;
                }
                else if (resp == Response::BACK) // вернуться обратно в игры
                {
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback();
                        --turn_num;
                    }
                    if (!beat_series)
                        --turn_num;

                    board.rollback();
                    --turn_num;
                    beat_series = 0;
                }
            }
            //Ход бота
            else
                bot_turn(turn_num % 2);
        }
        auto end = chrono::steady_clock::now(); // время окончания игры
        ofstream fout(project_path + "log.txt", ios_base::app); // сохранение лога игры
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n"; // подсчет времени игры
        fout.close();

        // Проверка выбора игрока
        if (is_replay) // попробовать ещё раз
            return play();
        if (is_quit) // выйти из игры
            return 0;
        int res = 2;
        if (turn_num == Max_turns) // все ходы были использованы (в случаях игры в дамках)
        {
            res = 0;
        }
        else if (turn_num % 2)
        {
            res = 1;
        }
        board.show_final(res); // показываем результат игры: кто победил
        auto resp = hand.wait();
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        return res;
    }

  private:
    void bot_turn(const bool color) // ход бота
    {
        auto start = chrono::steady_clock::now(); // время игры

        auto delay_ms = config("Bot", "BotDelayMS"); // задержка бота из файла settings.json
        // new thread for equal delay for each turn
        thread th(SDL_Delay, delay_ms); // создаём поток для бота
        auto turns = logic.find_best_turns(color); // вызываем функцию поиска лучшего хода
        th.join(); // выполняем ход за бота
        bool is_first = true;
        // making moves
        for (auto turn : turns)
        {
            if (!is_first)
            {
                SDL_Delay(delay_ms);
            }
            is_first = false;
            beat_series += (turn.xb != -1);
            board.move_piece(turn, beat_series);
        }

        auto end = chrono::steady_clock::now(); // время игры конечное
        ofstream fout(project_path + "log.txt", ios_base::app); // записываем лог
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close(); // закрываем поток файла лог
    }

    Response player_turn(const bool color) //ход пользователя
    {
        // return 1 if quit
        vector<pair<POS_T, POS_T>> cells; //список доступных ходов
        
        for (auto turn : logic.turns)
        {
            cells.emplace_back(turn.x, turn.y); // выделенная ячейка
        }
        board.highlight_cells(cells);
        move_pos pos = {-1, -1, -1, -1};
        POS_T x = -1, y = -1;
        // trying to make first move
        while (true)
        {
            auto resp = hand.get_cell();
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp);
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

            bool is_correct = false;

            // логика хождения игроком
            for (auto turn : logic.turns)
            {
                if (turn.x == cell.first && turn.y == cell.second) // проверка на корректный ход пользователем
                {
                    is_correct = true;
                    break;
                }
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn;
                    break;
                }
            }
            if (pos.x != -1)
                break;
            if (!is_correct)
            {
                if (x != -1)
                {
                    board.clear_active();
                    board.clear_highlight();
                    board.highlight_cells(cells);
                }
                x = -1;
                y = -1;
                continue;
            }
            x = cell.first;
            y = cell.second;
            board.clear_highlight();
            board.set_active(x, y);
            vector<pair<POS_T, POS_T>> cells2;
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2);
                }
            }
            board.highlight_cells(cells2);
        }
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1);
        if (pos.xb == -1)
            return Response::OK;
        // continue beating while can
        beat_series = 1;
        while (true)
        {
            logic.find_turns(pos.x2, pos.y2);
            if (!logic.have_beats)
                break;

            vector<pair<POS_T, POS_T>> cells;
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells);
            board.set_active(pos.x2, pos.y2);
            // trying to make move
            while (true)
            {
                auto resp = hand.get_cell();
                if (get<0>(resp) != Response::CELL)
                    return get<0>(resp);
                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

                bool is_correct = false;
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second)
                    {
                        is_correct = true;
                        pos = turn;
                        break;
                    }
                }
                if (!is_correct)
                    continue;

                board.clear_highlight();
                board.clear_active();
                beat_series += 1;
                board.move_piece(pos, beat_series);
                break;
            }
        }

        return Response::OK;
    }

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};

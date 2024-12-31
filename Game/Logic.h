#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
    }

    
    vector<move_pos> find_best_turns(const bool color)
    {
        next_best_state.clear(); // Очищаем массив значений лучших ходов
        next_move.clear(); // Очищаем массив значений следующий ход

        find_first_best_turn(board->get_board(), color, -1, -1, 0); 

        int state = 0; // текущее состояние
        vector<move_pos> result; // итоговый массив
        do // выполнить хотя бы 1 раз (точно) 
        {
            result.push_back(next_move[state]); // добавляем следующее состояние в конец
            state = next_best_state[state]; // присваиваем текущему состоянию следующее
        } while (state != -1 && next_move[state].x != -1);
        return result;
    }


private:
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const // выполнение хода: массив  ходов, ход игрока
    {
        if (turn.xb != -1) // если ход игрока != -1
            mtx[turn.xb][turn.yb] = 0;
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        mtx[turn.x][turn.y] = 0;
        return mtx; // возвращаем массив с ходом игрока
    }

    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const // функция подсчета отчков: передаем массив ячеек и цвет
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1);
                wq += (mtx[i][j] == 3);
                b += (mtx[i][j] == 2);
                bq += (mtx[i][j] == 4);
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        if (!first_bot_color) // если цвет бота не соответствует - меняем значения
        {
            swap(b, w);
            swap(bq, wq);
        }
        if (w + wq == 0)
            return INF;
        if (b + bq == 0)
            return 0;
        int q_coef = 4;
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5;
        }
        return (b + bq * q_coef) / (w + wq * q_coef);
    }

    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1) // поиск первого лучшего хода
    {
        next_best_state.push_back(-1); // добавляем в конец массив значение -1
        next_move.emplace_back(-1, -1, -1, -1); 
        double best_score = -1;
        if (state != 0) // если текущая позиция не равно 0, то вызываем функцию поиска хода
            find_turns(x, y, mtx);
        // иначе 
        auto turns_now = turns;
        bool have_beats_now = have_beats;

        if (!have_beats_now && state != 0) // Если отсутствует лучший ход и текущее состояние не равно 0, то вызываем функцию поиска хода
        {
            return find_best_turns_rec(mtx, 1 - color, 0, alpha);
        }

        vector<move_pos> best_moves;
        vector<int> best_states;

        for (auto turn : turns_now) // цикл ходов по всем возможным ходам
        {
            size_t next_state = next_move.size();
            double score;
            if (have_beats_now) // имеется ли сейчас возможность бить шашку противника
            {
                score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, next_state, best_score);
            }
            else // перемещаем шашку  
            {
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, 0, best_score);
            }
            if (score > best_score)
            {
                best_score = score;
                next_best_state[state] = (have_beats_now ? int(next_state) : -1);
                next_move[state] = turn;
            }
        }
        return best_score;
    }


    
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1)
    {
        if (depth == Max_depth) // если глубина биты равна максимальной бите
        {
            return calc_score(mtx, (depth % 2 == color));
        }
        if (x != -1) // ищем хода для шашки
        {
            find_turns(x, y, mtx);
        }
        else
            find_turns(color, mtx);
        auto turns_now = turns; // текущий ход
        bool have_beats_now = have_beats; // имеется ли сейчас возможность биты шашки

        if (!have_beats_now && x != -1) 
        {
            return find_best_turns_rec(mtx, 1 - color, depth + 1, alpha, beta);
        }

        if (turns.empty()) // если список ходов пустой
            return (depth % 2 ? 0 : INF);

        double min_score = INF + 1;
        double max_score = -1;
        for (auto turn : turns_now)
        {
            double score = 0.0;
            if (!have_beats_now && x == -1)
            {
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta);
            }
            else
            {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
            }
            min_score = min(min_score, score);
            max_score = max(max_score, score);
            // alpha-beta pruning
            if (depth % 2)
                alpha = max(alpha, max_score);
            else
                beta = min(beta, min_score);
            if (optimization != "O0" && alpha >= beta)
                return (depth % 2 ? max_score + 1 : min_score - 1);
        }
        return (depth % 2 ? max_score : min_score);
    }


public:
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board());
    }

    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board());
    }

private:
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx) // метод поиска хода
    {
        vector<move_pos> res_turns; // список ходов
        bool have_beats_before = false;
        // цикл ходов
        for (POS_T i = 0; i < 8; ++i) 
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (mtx[i][j] && mtx[i][j] % 2 != color) // если цвет ячейки не подходит
                {
                    find_turns(i, j, mtx);
                    if (have_beats && !have_beats_before) // можно ли бить шашку
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns; // массив ходов
        shuffle(turns.begin(), turns.end(), rand_eng);
        have_beats = have_beats_before;
    }

    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        turns.clear();
        have_beats = false;
        POS_T type = mtx[x][y];
        // check beats
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // check other turns
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns; // массив ходов
    bool have_beats;  // можно ли бить?
    int Max_depth; // максимальная глубина биты

  private:
    default_random_engine rand_eng;
    string scoring_mode; // режим подсчета очков (setting.json) 
    string optimization; // режим оптимизации (setting.json) 
    vector<move_pos> next_move; // массив следующего хода
    vector<int> next_best_state; // лучший ход
    Board *board; // указатель на игровое поле
    Config *config;// указатель на конфигурационная файл (setting.json) 
};

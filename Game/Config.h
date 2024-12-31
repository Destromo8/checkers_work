#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config()
    {
        reload();
    }

    void reload()
    {
        std::ifstream fin(project_path + "settings.json"); // открываем поток на чтение файла settings.json (файл с первоначальными настройками)
                                                           // project_path - это переменная директории проекта, описанный в файле Project_path.h в папке Models
        fin >> config; //Что было прочитано в файле сохраняем в переменную config с расширением json
        fin.close(); // закрываем поток чтения файла
    }

    auto operator()(const string &setting_dir, const string &setting_name) const // переопределение оператора (), чтобы передать туда два параметра путь до директории и название файла настроект
    {
        return config[setting_dir][setting_name]; // возвращает переменную config с веденными параметрами
    }

  private:
    json config;
};

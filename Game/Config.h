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
        std::ifstream fin(project_path + "settings.json"); // ��������� ����� �� ������ ����� settings.json (���� � ��������������� �����������)
                                                           // project_path - ��� ���������� ���������� �������, ��������� � ����� Project_path.h � ����� Models
        fin >> config; //��� ���� ��������� � ����� ��������� � ���������� config � ����������� json
        fin.close(); // ��������� ����� ������ �����
    }

    auto operator()(const string &setting_dir, const string &setting_name) const // ��������������� ��������� (), ����� �������� ���� ��� ��������� ���� �� ���������� � �������� ����� ���������
    {
        return config[setting_dir][setting_name]; // ���������� ���������� config � ��������� �����������
    }

  private:
    json config;
};

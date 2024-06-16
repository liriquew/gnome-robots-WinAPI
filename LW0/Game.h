#pragma once
#include "framework.h"

class Game
{
private:
    // переменные для корректного отображения
    float aspectRatio;
    int count_hor, count_ver;
    int cl_size;
    int bttn_size;

    int fieldX, fieldY;
    int fieldWidth, fieldHeight;

    float scaleX, scaleY;


    std::vector<POINT> dirs;

    // переменные для игры
    int diff_level;
    int sum_score;
    POINT player_position;
    POINT old_player_position;
    std::vector<POINT> units;
    std::vector<POINT> old_units;
    std::vector<POINT> dead_units;
    HBITMAP& BG_bitmap;
    HBITMAP& CH_bitmap;
    HWND& hwnd;
    int start_level;

    POINT GetRandPoint();

    void CalculateNewUnitPos();

    void ShowField(HDC hDC, int windowWidth, int windowHeight);
    
    bool Check();

    void SetUnitPos();
    
    POINT GetCHposition(int windowWidth, int windowHeight);

public:

    Game(int count_hor, int count_ver, int cl_size, int bttn_size, int diff_level, HWND& hwnd, HBITMAP& BG_bitmap, HBITMAP& CH_bitmap);

    ~Game();

    void Repaint(HDC hDC, int windowWidth, int windowHeight, bool repaint = false);
    
    void TestMethod(HDC hDC, POINT point);

    void Move(POINT point = { -1, -1 });

    short int GetDir(POINT point);

    POINT GetCharacterPos();

    POINT GetRel(POINT curs_point);

    void SetNewRandPosition();
    
    short int GameStatus();

    void Restart(short int gm_code, int diff_level = -1);

    int GetLevel();

    int GetScore();
};

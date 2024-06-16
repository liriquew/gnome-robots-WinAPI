#include "Game.h"
#include "Resource.h"

static std::vector<POINT> dirs;             // направления движений
// минимальная дистанция от персонажа пользователя до юнита,
// при которой ход будет считаться допустимым
// (+0.0001 так как sqrt вычисляет с погрешностью)
const float min_dir = sqrt(2) + 0.01;

// конструктор
Game::Game(int count_hor, int count_ver, int cl_size, int bttn_size, int diff_level, HWND& hwnd, HBITMAP& BG_bitmap, HBITMAP& CH_bitmap)
    : hwnd(hwnd), BG_bitmap(BG_bitmap), CH_bitmap(CH_bitmap) // инициализация ссылок
{
    this->player_position = { count_hor / 2, count_ver / 2};
    this->aspectRatio = (float)count_hor * cl_size / (count_ver * cl_size + bttn_size);
    this->bttn_size = bttn_size;
    this->count_hor = count_hor;
    this->count_ver = count_ver;
    this->cl_size = cl_size;

    // размеры "заглушек"
    this->fieldX = 0;
    this->fieldY = 0;

    this->fieldWidth = count_hor * cl_size;
    this->fieldHeight = count_ver * cl_size;

    // размеры клеточек
    this->scaleX = (float)cl_size;
    this->scaleY = (float)cl_size;

    // актуальные позиции юнитов
    this->units = std::vector<POINT>(diff_level * 10 + 10, { 0, 0 });
    // пустой вектор (нужен для сохранения позиций в которых надо будет закрасить)
    this->old_units = std::vector<POINT>(0);

    this->diff_level = diff_level;      // текущий уровень
    this->start_level = diff_level;     // начальный уровень

    this->sum_score = 0;

    if (dirs.size() == 0)
        dirs = std::vector<POINT>({ {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {0, -1}, {1, -1}, {1, 0}, {1, 1} });
    SetUnitPos();   // установим случайные позиции игроков
}

Game::~Game() {
    this->units.clear();
    this->old_units.clear();
    this->dead_units.clear();
    dirs.clear();
}

// метод для генерации координат случайной точки
POINT Game::GetRandPoint() {
    int x, y;
    while (true) {
        x = rand() % count_hor;
        y = rand() % count_ver;
        // т.о. исключим возможность снегерировать точку, в которой находится игрок
        if (x != player_position.x && y != player_position.y) {
            break;
        }
    }
    return {x, y};
}

// метод для установки случайных позиций юнитов
void Game::SetUnitPos() {
    POINT rand_point;
    int i = 0;
    while (true) {                                          // генерация изначальной позиции врагов
        rand_point = GetRandPoint();
        if (std::find_if(units.begin(), units.end(), [rand_point](POINT point) {
                return rand_point.x == point.x && rand_point.y == point.y; 
            }) == units.end()               // т.е. если прошли весь вектор и не нашли совпадение
            ) {                            // то есть если эта точка не была получена ранее
            units[i] = rand_point;
            i++;
            if (i == units.size()) {
                break;
            }
        }
    }
}

// метод для отображения игрового поля (фона)
void Game::ShowField(HDC hDC, int windowWidth, int windowHeight) {
    HDC hMemDC;                         // совместимый контекст устройства
    BITMAP bm;                          // битмапа (получим позже по дискриптору битмапы)

    hMemDC = CreateCompatibleDC(hDC);   // создаем совместимый контекст устройства
    SelectObject(hMemDC, BG_bitmap);
    GetObject(BG_bitmap, sizeof(BITMAP), (LPSTR)&bm);

    if ((float)windowWidth / windowHeight < aspectRatio) {  // окно шире, чем нужно, поэтому ширина поля должна занимать всю ширину окна
        fieldWidth = windowWidth;
        fieldHeight = (int)(windowWidth / aspectRatio);
    }
    else {                                                  // окно выше, чем нужно, поэтому высота поля должна занимать всю высоту окна
        fieldWidth = (int)(windowHeight * aspectRatio);
        fieldHeight = windowHeight;
    }

    // размеры "заглушек" по бокам
    fieldX = (windowWidth - fieldWidth) / 2;
    fieldY = (windowHeight - fieldHeight) / 2;

    fieldHeight -= bttn_size; // учли, что внизу кнопки

    StretchBlt(hDC, fieldX, fieldY, fieldWidth, fieldHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    DeleteDC(hMemDC);
}

// метод для перерисовывания позиций юнитов при шаге
void Game::Repaint(HDC hDC, int windowWidth, int windowHeight, bool repaintField) {
    if (repaintField) {
        ShowField(hDC, windowWidth, windowHeight);
    }

    int x, y, unitSize, x_src, y_src;
    HDC hMemDC;
    BITMAP bm;
    
    // получим параметры масштабирования при выводе в отдельную ячейку
    scaleX = cl_size * (float)fieldWidth / (count_hor * cl_size);
    scaleY = cl_size * (float)fieldHeight / (count_ver * cl_size);

    hMemDC = CreateCompatibleDC(hDC);                   // создаем совместимый контекст устройства
    SelectObject(hMemDC, BG_bitmap);
    GetObject(BG_bitmap, sizeof(BITMAP), (LPSTR)&bm);

    // закрашиваем предыдущие позиции юнитов
    for (POINT p : old_units) {
        // fieldX, fieldY - размеры заглушек
        x = fieldX + (int)(p.x * scaleX);
        y = fieldY + (int)(p.y * scaleY);
        unitSize = (int)scaleY;
        x_src = cl_size * ((p.x + p.y) % 2 != 0);
        y_src = cl_size * ((p.x + p.y) % 2 != 0);
        StretchBlt(hDC, x, y, unitSize, unitSize, hMemDC, x_src, 0, cl_size, cl_size, SRCCOPY);
    }
    
    // закрашиваем предыдущую позицию игрока
    x_src = cl_size * ((old_player_position.x + old_player_position.y) % 2 != 0);
    StretchBlt(hDC, (int)(old_player_position.x * scaleX) + fieldX, (int)(old_player_position.y * scaleY) + fieldY, 
                    (int)scaleX, (int)scaleY, hMemDC, x_src, 0, cl_size, cl_size, SRCCOPY);
    DeleteDC(hMemDC);

    // отрисовываем новые позиции

    hMemDC = CreateCompatibleDC(hDC);   // создаем совместимый контекст устройства
    SelectObject(hMemDC, CH_bitmap);
    GetObject(CH_bitmap, sizeof(BITMAP), (LPSTR)&bm);

    // отображение позиции игрока
    TransparentBlt(hDC, player_position.x * scaleX + fieldX, player_position.y * scaleY + fieldY, scaleX, scaleX, hMemDC, 0, 0, 30, 30, RGB(255, 255, 255));

    for (POINT p : units) {             // отрисуем юнитов
        x = fieldX + p.x * scaleX;
        y = fieldY + p.y * scaleY;
        unitSize = scaleY;
        
        TransparentBlt(hDC, x, y, unitSize, unitSize, hMemDC, 60, 0, 30, 30, RGB(255, 255, 255));  // отображение прочих юнитов (такая функция чтобы убрать белый фон)
    }

    for (POINT p : dead_units) {        // отрисуем позиции в которых ранее уничтожались юниты
        x = fieldX + p.x * scaleX;
        y = fieldY + p.y * scaleY;
        unitSize = scaleY;
        
        TransparentBlt(hDC, x, y, unitSize, unitSize, hMemDC, 150, 0, 30, 30, RGB(255, 255, 255)); // отображение прочих юнитов
    }

    DeleteDC(hMemDC);
}

// методы для получения координат позиции игрока и произвольной точки в экранных единицах и наоборот (не используется)
/*
POINT Game::GetCHposition(int windowWidth, int windowHeight) {

    if ((float)windowWidth / windowHeight < aspectRatio) {  // окно шире, чем нужно, поэтому ширина поля должна занимать всю ширину окна
        fieldWidth = windowWidth;
        fieldHeight = (int)(windowWidth / aspectRatio);
    }
    else {                                                  // окно выше, чем нужно, поэтому высота поля должна занимать всю высоту окна
        fieldWidth = (int)(windowHeight * aspectRatio);
        fieldHeight = windowHeight;
    }

    // размеры "заглушек" по бокам
    fieldX = (windowWidth - fieldWidth) / 2;
    fieldY = (windowHeight - fieldHeight) / 2;

    fieldHeight -= bttn_size;

    scaleX = cl_size * (float)fieldWidth / (count_hor * cl_size);
    scaleY = cl_size * (float)fieldHeight / (count_ver * cl_size);

    POINT player_pos;
    player_pos.x = fieldX + player_position.x * scaleX;
    player_pos.y = fieldY + player_position.y * scaleY;

    return player_pos;
}


POINT Game::GetCharacterPos() {
    return player_position;
}

POINT Game::GetRel(POINT curs_point) {
    return { (curs_point.x - fieldX) / cl_size, (curs_point.y - fieldY) / cl_size};
}
*/

// метод для определения направления движения относительно игрока
short int Game::GetDir(POINT point) {
    // так как получили в экранных единицах, преобразуем в единицы "поля"
    point.x = (point.x - fieldX) * count_hor / fieldWidth;
    point.y = (point.y - fieldY) * count_ver / fieldHeight;

    short int current_rel = 0;              // закодируем направление движения (Resource.h)
    if (player_position.x > point.x)
        current_rel |= 1;
    else if (player_position.x < point.x)
        current_rel |= 2;
    else
        current_rel |= 4;

    if (player_position.y > point.y)
        current_rel |= 8;
    else if (player_position.y < point.y)
        current_rel |= 16;
    else
        current_rel |= 32;
    return current_rel;
}

// метод для вычисления новых позиций юнитов
void Game::CalculateNewUnitPos() {
    old_units = std::vector<POINT>(units);  // чтобы закрасить

    int x, y;
    int purp_dir;

    for (int i = 0; i < units.size(); i++) {
        float min_dist = -1;
        for (int j = 0; j < dirs.size(); j++) {             // обходим окресность юнита
            // берем расстояния от юнита до игрока при направлении dir[j]
            x = units[i].x + dirs[j].x - player_position.x;
            y = units[i].y + dirs[j].y - player_position.y;

            if (min_dist == -1 || min_dist > sqrt(x * x + y * y)) {   // и ищем минимум евклидовой нормы
                min_dist = sqrt(x * x + y * y);
                purp_dir = j;
            }
        }
        // обновляем позицию юнита при найденном направлении dirs[purp_dir]
        units[i] = { units[i].x + dirs[purp_dir].x, units[i].y + dirs[purp_dir].y };
    }
}

// метод для шага
void Game::Move(POINT point) {
    // обновим позицию игрока в соответствии с направлением шага
    short int current_dir = 0;

    if (point.x != -1 && point.y != -1) {   // в противном случае была нажата кнопка "Подождать"
        current_dir = GetDir(point);
        POINT buf = player_position;        // возьмем позицию игрока и будем работать с копией

        switch (current_dir)                // сделаем шаг "игрока"
        {
            case POS_NO: {break; }
            case POS_LT: {buf.x -= 1; buf.y -= 1; break; }
            case POS_LL: {buf.x -= 1; break; }
            case POS_LB: {buf.x -= 1; buf.y += 1; break; }
            case POS_TT: {buf.y -= 1; break; }
            case POS_BB: {buf.y += 1; break; }
            case POS_RT: {buf.x += 1; buf.y -= 1; break; }
            case POS_RR: {buf.x += 1; break; }
            case POS_RB: {buf.x += 1; buf.y += 1; break; }
        }

        // проверим безопасность такого хода
        // границы
        if (buf.x < 0 || buf.x > (count_hor - 1) || buf.y < 0 || buf.y > (count_ver - 1)) {
            return;
        }

        // если при таком шаге следующий ход проигрышный
        for (int i = 0; i < units.size(); i++) {

            if (sqrt(pow(units[i].x - buf.x, 2) + pow(units[i].y - buf.y, 2)) <= min_dir)
                return;
        }

        // игрок не может сделать шаг в позиции, в которых ранее уничтожались юниты
        for (std::vector<POINT>::iterator i = dead_units.begin(); i != dead_units.end(); i++) {
            if (buf.x == i->x && buf.y == i->y)
                return;
        }
        if (current_dir != (POS_NO))       // если не стояли на месте (иначе закрасили бы)
            old_player_position = player_position;
        player_position = buf;              // обновим позицию игрока после всех проверок
    }
    // теперь юниты делают шаг
    CalculateNewUnitPos();
    // проверим позиции юнитов, возмножно некоторые из них теперь уничтожены
    Check();
}

// метод для случайной телепортации
void Game::SetNewRandPosition() {
    POINT point;
    while (true)
    {
        point = GetRandPoint();
        if (point.x != player_position.x && point.y != player_position.y) {
            old_player_position = player_position;  // чтобы закрасить
            player_position = point;
            break;
        }
    }
    this->Move();
}

// метод для проверки позиций юнитов, и для формирования dead_units
bool Game::Check() {
    // перебираем все возможные пары юнитов
    // если совпадение в паре, то добавляем в dead_units
    for (std::vector<POINT>::iterator i = units.begin(); i != units.end(); i++) {
        for (std::vector<POINT>::iterator j = i + 1; j != units.end();) {
            if (i->x == j->x && i->y == j->y) {
                dead_units.push_back(*i);
                i = units.erase(i);
                j = i + 1;
            }
            else {
                j++;
            }
        }
    }

    // если юнит попал на "разрушенного" юнита
    // также перебираем пары (разрушенный юнит, юнит)
    // если совпадение, то удаляем этого юнита
    for (std::vector<POINT>::iterator i = dead_units.begin(); i != dead_units.end(); i++) {
        for (std::vector<POINT>::iterator j = units.begin(); j != units.end();) {
            if (i->x == j->x && i->y == j->y) {
                if (units.size() != 1) {
                    j = units.erase(j);
                }
                else {
                    units.clear();
                    return true;
                }
            }
            else {
                j++;
            }
        }
    }

    return units.empty();
}

// метод для получения статуса игры (GM_WIN, GM_CONTINUE, GM_LOSE)
short int Game::GameStatus() {
    if (units.empty()) {
        return GM_WIN;
    }
    POINT buf;
    POINT dist;
    short int count_valid_dirs = 8; 

    for (POINT dir : dirs) {    // перебираем возможные направления движения игрока
        buf = player_position;
        buf.x += dir.x;
        buf.y += dir.y;
        if (buf.x < 0 || buf.x > count_hor || buf.y < 0 || buf.y > count_ver) { // проверяем выход за границы
            count_valid_dirs--;
            continue;
        }
        for (POINT unit : units) {      // перебираем вражеских юнитов
            // дистанция от предполагаемого положения игрока при ходе dir до юнита unit
            dist.x = buf.x - unit.x;    
            dist.y = buf.y - unit.y;
            if (abs(dist.x) <= 1 && abs(dist.y) <= 1) {
                count_valid_dirs--; // уменьшим число возможных путей для хода, так как при таком ходе проиграем
                break;              // отбрасываем текущее направление движения
            }
        }
    }
    return count_valid_dirs == 0 ? GM_LOSE : GM_CONTINUE;
}

// метод для перезапуска игры
void Game::Restart(short int gm_code, int diff_level) {
    if (diff_level != -1) {     // если перезапуск из-за поражения / победы
        this->sum_score = 0;
        this->diff_level = diff_level - 1;
        this->start_level = diff_level;
        this->sum_score = 0;
    }

    if (gm_code == GM_WIN) {     // все равно когда рестарт сюда попадем
        this->diff_level++;
    }
    else {                       // если поражение, то сбрасываем на начальный уровень
        this->diff_level = start_level;
        this->sum_score = 0;
    }
        
    this->player_position = { count_hor / 2, count_ver / 2 };
    dead_units.clear();
    units.resize(10 + 10 * this->diff_level);

    SetUnitPos();
}

int Game::GetLevel() {
    return diff_level + 1;
}

int Game::GetScore() {
    // пройденный уровень - 100
    // побежденный юнит - 10
    // разрушение - 5
    // + счет в предыдущих уровнях
    return (diff_level - start_level) * 100 + (10 + diff_level * 10 - units.size()) * 10 + dead_units.size() * 5 + this->sum_score;
}
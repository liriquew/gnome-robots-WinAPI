#include "framework.h"
#include "resource.h"
#include "Game.h"
#include "LW0.h"

#define MAX_LOADSTRING 100
#define TITLE_LOADSTRING 25
#define _CRT_SECURE_NO_WARNINGS

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
TCHAR szWindowChildClass[MAX_LOADSTRING];       // имя класса дочернего окна.

HWND hwnd;          // главное окно приложения
HWND hwndsttngs;    // дочернее окно с настройками

HWND RandTP_Bttn, Info, Wait_Bttn;              // элементы в главном окне (панель снизу)

HWND hList, hEdt, husername, hCmbBx;            // элементы в дочернем окне
HWND hChBx, hRBttn1, hRBttn2, hSvNChBx;

// размеры игрового поля в "клеточках"
const int cl_count_hor = 40;
const int cl_count_ver = 30;
const int cl_size = 20;

int bttns_vert_size = 70;   // высота в пикселях нижней панели
int min_width, min_height;  // минимальная ширина и высота главного окна (определяются при создании окна)

const float aspectRatio = (float)cl_count_hor * cl_size/(cl_count_ver * cl_size + bttns_vert_size); // соотношение сторон

// дескрипторы битовых карт
HBITMAP Btmp_BG = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_BG));    // игровое поле
HBITMAP Btmp_CH = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_CH));    // игровые персонажи

Game* G;            // объект игры
bool repaint_bg;    // флаг для перерисовываения заднего фона
POINT click_point;  // точка в которой был совершен клик по игровому полю

TCHAR username[MAX_LOADSTRING]; // имя пользователя (вводится в окне настроек)

// звуки игры
bool SoundPlay = false;             // проигрывание звука
short int Selected = -1;            // выбранный звук
TCHAR SoundTitleF[TITLE_LOADSTRING];
TCHAR SoundTitleS[TITLE_LOADSTRING];
TCHAR Sound_TitleGO[MAX_LOADSTRING];

const RECT settingsStartPos = { 100, 100, 700, 360 };

// для корректного отображения курсора
int cursorPos = -1;     // текущее направление в котором расположен курсор
int cursor_index = -1;  // индекс курсора
RECT curRECT;           // прямоугольник, ограничивающий игровое поле
POINT pt;               // точка в оконных координатах, в которой находится курсор
std::vector<HCURSOR> cursors;   // курсоры

// функция для загрузки курсоров
void LoadCursors() {
    for (int i = IDC_CURNO; i <= IDC_CURPL; i++)
        cursors.push_back(LoadCursor(hInst, MAKEINTRESOURCE(i)));
}

// функция для добавления в файл новой записи об игре
void AddScore(wchar_t name[], int score) {
    FILE* file = _wfopen(TEXT(FILENAME), _T("at, ccs=UTF-16LE"));      // открываем файл на дозапись
    if (file == NULL) {
        return;
    }
    fwprintf(file,TEXT("%ls, %d\n"), name, score);
    fclose(file);
}

// функция для загрузки результатов игр из файла
std::vector<std::wstring> loadScores() {
    std::vector<std::wstring> players;
    FILE* file = _wfopen(TEXT(FILENAME), _T("rt, ccs=UTF-16LE"));
    if (file == NULL) {
        return players;
    }
    wchar_t line[MAX_LOADSTRING];
    while (fgetws(line, MAX_LOADSTRING, file) != NULL) {
        players.push_back(line);
    }
    fclose(file);
    return players;
}

// функция для определения структуры RECT содержащей информацию об игровом поле
RECT GetGameRect() {
    RECT rect;
    GetClientRect(hwnd, &rect);
    rect.bottom -= bttns_vert_size;
    return rect;
}

// функция для обновления информации об игре в окне
void UpdateInfo() {
    int level = G->GetLevel(); 
    int score = G->GetScore();
    
    TCHAR level_tch[10], score_tch[10];

    _itot_s(level, level_tch, 10);
    _itot_s(score, score_tch, 10);

    TCHAR Title[MAX_LOADSTRING] = _T("Счет: ");
    _tcscat_s(Title, score_tch);
    _tcscat_s(Title, _T(" Уровень: "));
    _tcscat_s(Title, level_tch);
    SetWindowText(Info, Title);
}

// функция для шага и отображения актуальных позиций юнитов
void MakeMove(POINT click_point = {-1, -1}) {
    // проигрывание звука
    if (SoundPlay && Selected > 0) {
        PlaySound(Selected == 1 ? SoundTitleF : SoundTitleS, NULL, SND_FILENAME | SND_ASYNC);
    }
    G->Move(click_point);       // ход
    RECT rect = GetGameRect();
    InvalidateRect(hwnd, &rect, false);     // отображение новых позиций после возврата

    short int status = G->GameStatus();     // берем статус игры
    if (status != GM_CONTINUE) {            // если GM_WIN, или GM_LOSE
        KillTimer(hwnd, ID_WAIT_TIMER);     // обработка, если была нажата кнопка 'Подождать'
        // отправляем сообщение с соответствующим кодом в окно с игрой
        SendMessage(hwnd, WM_COMMAND, status, -1);
        // отрисуем поле заново
        repaint_bg = true;
    }
    // обновим информацию в нижнем меню
    UpdateInfo();
}

// Отправить объявления функций, включенных в этотv модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyChildClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndChildProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndEndGameProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LW0, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHILD, szWindowChildClass, MAX_LOADSTRING);  // заголовок дочернего окна
    LoadStringW(hInstance, SND_SOUND1, SoundTitleF, TITLE_LOADSTRING);      // путь к файлу с звуком 1
    LoadStringW(hInstance, SND_SOUND2, SoundTitleS, TITLE_LOADSTRING);      // путь к файлу с звуком 2
    LoadStringW(hInst, SND_GAMEOVER, Sound_TitleGO, MAX_LOADSTRING);        // звук проигрыша

    // регистрация классов окон
    MyRegisterClass(hInstance);
    MyChildClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LW0));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

// регистрирует класс родительского окна с игрой
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LW0));
    wcex.hCursor        = NULL;     // чтобы курсор не восстанавливался при каждом перемещении мыши
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LW0);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

// регистрирует класс дочернего окна с настройками
ATOM MyChildClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndChildProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LW0));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowChildClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // сохранение маркера экземпляра в глобальной переменной

   RECT clientRect = { 0, 0, cl_count_hor * cl_size, cl_count_ver * cl_size + bttns_vert_size};

   AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW, TRUE, 0);

   // Теперь clientRect содержит размеры окна, учитывая рамки и заголовок
   min_width = clientRect.right - clientRect.left;
   min_height = clientRect.bottom - clientRect.top;

   hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, min_width, min_height, nullptr, nullptr, hInstance, nullptr);
   
   if (!hwnd)
      return FALSE;

   LoadCursors();

   hwndsttngs = CreateWindow(szWindowChildClass, szWindowChildClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 700, 330, hwnd, NULL, hInstance, NULL);

   if (!hwndsttngs)
       return FALSE;

   G = new Game(cl_count_hor, cl_count_ver, cl_size, bttns_vert_size, 0, hwnd, Btmp_BG, Btmp_CH);

   // родительское окно
   RandTP_Bttn = CreateWindow(TEXT("BUTTON"), TEXT("Случайная телепортация"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, cl_count_ver * cl_size, min_width / 3, bttns_vert_size, hwnd, (HMENU)ID_RANDTP_BTTN, hInstance, NULL);
   Info = CreateWindow(TEXT("STATIC"), TEXT("Счет: 0 Уровень: 1"), WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_CENTER, min_width / 3, cl_count_ver * cl_size, min_width / 3, bttns_vert_size, hwnd, (HMENU)ID_TABLE_STTC, hInstance, NULL);
   Wait_Bttn = CreateWindow(TEXT("BUTTON"), TEXT("Подождать"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, min_width * 2 / 3, cl_count_ver * cl_size, min_width / 3, bttns_vert_size, hwnd, (HMENU)ID_WAIT_BTTN, hInstance, NULL);

   // дочернее окно
   CreateWindow(TEXT("STATIC"), TEXT("История игр"), WS_VISIBLE | WS_CHILD, 400, 10, 100, 30, hwndsttngs, (HMENU)ID_STATIC, hInst, NULL);
   hList = CreateWindow(L"LISTBOX", TEXT(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER, 400, 30, 270, 280, hwndsttngs, (HMENU)ID_LISTBOX, hInstance, NULL);

   CreateWindow(TEXT("STATIC"), TEXT("Уровень сложности"), WS_VISIBLE | WS_CHILD, 10, 10, 200, 30, hwndsttngs, (HMENU)ID_STATIC, hInst, NULL);
   hCmbBx = CreateWindow(L"COMBOBOX", NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 10, 30, 270, 100, hwndsttngs, (HMENU)ID_COMBOBOX, GetModuleHandle(NULL), NULL);

   CreateWindow(TEXT("STATIC"), TEXT("Настройки звука"), WS_VISIBLE | WS_CHILD, 10, 70, 200, 30, hwndsttngs, (HMENU)ID_STATIC, hInst, NULL);
   hChBx = CreateWindow(TEXT("BUTTON"), TEXT("Включить звук"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BST_CHECKED, 10, 90, 150, 30, hwndsttngs, (HMENU)ID_CHECKBOX, hInst, NULL);
   hRBttn1 = CreateWindow(TEXT("BUTTON"), TEXT("Звук 1"), WS_CHILD | BS_AUTORADIOBUTTON, 25, 120, 150, 30, hwndsttngs, (HMENU)ID_RADIO_BTTN_F, hInst, NULL);
   hRBttn2 = CreateWindow(TEXT("BUTTON"), TEXT("Звук 2"), WS_CHILD | BS_AUTORADIOBUTTON, 25, 150, 150, 30, hwndsttngs, (HMENU)ID_RADIO_BTTN_S, hInst, NULL);

   CreateWindow(TEXT("STATIC"), TEXT("Введите ваше имя"), WS_VISIBLE | WS_CHILD, 10, 190, 200, 30, hwndsttngs, (HMENU)ID_USERNAME_STTC, hInst, NULL);
   hEdt = CreateWindow(TEXT("EDIT"), TEXT(""), ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 210, 200, 30, hwndsttngs, (HMENU)ID_USERNAME_EDIT, hInst, NULL);
   hSvNChBx = CreateWindow(TEXT("BUTTON"), TEXT("Сохранить"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BST_CHECKED, 230, 210, 100, 30, hwndsttngs, (HMENU)ID_SAVENICKNAME, hInstance, NULL);

   // добавляем в ComboBox элементы (уровни сложности)
   SendMessage(hCmbBx, CB_ADDSTRING, 0, (LPARAM)TEXT("Легкий"));
   SendMessage(hCmbBx, CB_ADDSTRING, 0, (LPARAM)TEXT("Средний"));
   SendMessage(hCmbBx, CB_ADDSTRING, 0, (LPARAM)TEXT("Сложный"));
   SendMessage(hCmbBx, CB_SETCURSEL, 0, 0); // текущий выбранный элемент - 0 (легкий)

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);
   UpdateWindow(hwndsttngs);

   return TRUE; 
}

// оконная процедура главного окна с игрой
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SendMessage(hWnd, SBM_SETPOS, 1, 1);
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:{
                // модальное
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break; 
            }
            case IDM_DIALOG:{
                // немодальное
                CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG), hWnd, WndEndGameProc);
                break;
            }
            case IDM_SETTINGS: {    // отображение окна с настройками
                SetWindowPos(hwndsttngs, hwnd, settingsStartPos.left, settingsStartPos.top, settingsStartPos.right, settingsStartPos.bottom, 0);
                ShowWindow(hwndsttngs, SW_SHOWNORMAL);
                break;
            }
            case IDM_METAFILE: {
                HDC metaFile = CreateMetaFile(L"metafile.wmf");                 // создаем метафайл и получаем его контекст устройства
                HBRUSH br_1 = CreateSolidBrush(RGB(117, 144, 174));     // создаем сплошную кисть
                HBRUSH br_2 = CreateSolidBrush(RGB(105, 130, 157));     // создаем сплошную кисть

                for (int i = 0; i < cl_count_hor; i++) {
                    for (int j = 0; j < cl_count_ver; j++) {
                        RECT cellRect = { i * cl_size, j * cl_size, (i + 1) * cl_size, (j + 1) * cl_size };
                        if ((i + j) % 2 == 0) {
                            FillRect(metaFile, &cellRect, br_1);
                        }
                        else {
                            FillRect(metaFile, &cellRect, br_2);
                        }
                    }
                }

                ReleaseDC(hWnd, metaFile);
                HMETAFILE hmf = CloseMetaFile(metaFile);
                DeleteDC(metaFile);
                break;
            }
            case GM_WIN: {  // победа
                if (lParam == -1) {     // если сообщение пришло при завершении игры
                    G->Restart(GM_WIN);
                }
                else {                  // если сообщение пришло при изменении уровня сложности
                    G->Restart(GM_WIN, lParam);
                }
                // перерисовываем игровое поле
                repaint_bg = true;
                InvalidateRect(hWnd, NULL, false);
                UpdateInfo();
                break;
            }
            case GM_LOSE: { // поражение
                if (SoundPlay) {
                    PlaySound(Sound_TitleGO, hInst, SND_FILENAME | SND_ASYNC);
                }
                // если имя игрока по умолчанию не было введено
                if (SendMessage(hSvNChBx, BM_GETCHECK, 0, 0) == BST_UNCHECKED && lstrlen(username) == 0) {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG), hWnd, WndEndGameProc);
                }

                if (lstrlen(username) > 0) {
                    AddScore(username, G->GetScore());  // добавление записи
                    ShowWindow(hwndsttngs, SW_SHOW);    // отображение окна с настройками
                }

                G->Restart(GM_LOSE);    // перезапуск игры

                repaint_bg = true;      // флаг для перерисовывания игрового поля
                InvalidateRect(hWnd, NULL, false);  // перерисовывание окна
                UpdateInfo();                   // обновление информации внизу окна
                break;
            }
            case ID_RANDTP_BTTN: {
                G->SetNewRandPosition();        // случайное перемещение
                
                int status = G->GameStatus();   // проверка статуса игры (можно телепортироваться на юнита)
                if (status != GM_CONTINUE) {    // если GM_WIN или GM_LOSE
                    KillTimer(hWnd, ID_WAIT_BTTN);              // если была нажата кнопка 'Подождать'
                    SendMessage(hwnd, WM_COMMAND, status, 0);   // перезапуск
                    break;
                }
                else {                          // в противном случае отрисовывание новых позиций
                    RECT rect = GetGameRect();
                    InvalidateRect(hWnd, &rect, false);
                }
                break;
            }
            case ID_WAIT_BTTN: {    // обработка нажатия кнопки для ожидания движения вражеских персонажей
                SetTimer(hWnd, ID_WAIT_TIMER, 200, NULL);
                break;
            }
            case IDM_EXIT: {
                DestroyWindow(hwndsttngs);
                DestroyWindow(hWnd);
                break; 
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_MOUSEMOVE: {
        GetClientRect(hwnd, &curRECT); 
        curRECT.bottom -= bttns_vert_size;
        
        // получим точку в клиентской системе координат
        pt = { LOWORD(lParam) - curRECT.left + 10, HIWORD(lParam) - curRECT.top + 10};


        if (PtInRect(&curRECT, pt)) {   // проверяем, находится ли курсор в игровом поле
            short int current_rel = G->GetDir(pt);  // получим направление
            if (current_rel != cursorPos) { // будем менять курсор, только если надо
                cursorPos = current_rel;
            }
            else {
                SetCursor(cursors[cursor_index]);
                return 0;
            }

            switch (current_rel) {
                case POS_NO: {SetCursor(cursors[0]); cursor_index = 0; break; }
                case POS_LT: {SetCursor(cursors[1]); cursor_index = 1; break; }
                case POS_LL: {SetCursor(cursors[3]); cursor_index = 3; break; }
                case POS_LB: {SetCursor(cursors[2]); cursor_index = 2; break; }
                case POS_TT: {SetCursor(cursors[4]); cursor_index = 4; break; }
                case POS_BB: {SetCursor(cursors[5]); cursor_index = 5; break; }
                case POS_RT: {SetCursor(cursors[6]); cursor_index = 6; break; }
                case POS_RR: {SetCursor(cursors[7]); cursor_index = 7; break; }
                case POS_RB: {SetCursor(cursors[8]); cursor_index = 8; break; }
                default: {SetCursor(cursors[9]); cursor_index = 9; }
            }
        }
        else {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        break;
    }
    case WM_TIMER: {    // обработка таймера для кнопки ожидания юнитов
        if (wParam == ID_WAIT_TIMER)
            MakeMove(); // делаем шаг по таймеру
        break;
    }
    case WM_LBUTTONDOWN: {
        click_point = { LOWORD(lParam), HIWORD(lParam) };   // "запоминаем" где была нажата лкм
        break;
    }
    case WM_LBUTTONUP: {
        // если лкм отпущена там же где и нажата
        if (click_point.x == LOWORD(lParam) && click_point.y == HIWORD(lParam)) {
            RECT rect = GetGameRect();
            if (PtInRect(&rect, click_point)) {         // если "клик" на игровом поле
                MakeMove({ click_point.x + 10, click_point.y - 10 });   // делаем шаг
            }
        }
        break;
    }
    case WM_GETMINMAXINFO: {    // установка минимальных размеров окна (приходит при каждом изменении размеров окна)
        MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;
        minMaxInfo->ptMinTrackSize.x = min_width;
        minMaxInfo->ptMinTrackSize.y = min_height;
        break;
    }
    case WM_SIZE: { // установка размеров элементов на нижней панели при изменении размеров окна
        RECT rect;
        GetClientRect(hWnd, &rect);

        SetWindowPos(RandTP_Bttn, hWnd, 0, rect.bottom - rect.top - bttns_vert_size, (rect.right - rect.left) / 3, bttns_vert_size, SWP_NOZORDER);
        SetWindowPos(Info, hWnd, (rect.right - rect.left) / 3, rect.bottom - rect.top - bttns_vert_size, (rect.right - rect.left) / 3, bttns_vert_size, SWP_NOZORDER);
        SetWindowPos(Wait_Bttn, hWnd, (rect.right - rect.left) * 2 / 3, rect.bottom - rect.top - bttns_vert_size, (rect.right - rect.left) / 3, bttns_vert_size, SWP_NOZORDER);
        
        repaint_bg = true;  // также, нужно заново отобразить поле
        break;
    }
    case WM_RBUTTONDOWN: {  // по нажатию пкм будет отображено POPUP окно с меню
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, IDM_SETTINGS, TEXT("Настройки"));
        AppendMenu(hMenu, MF_STRING, IDM_DIALOG, TEXT("Ввести имя"));
        POINT p;
        p.x = LOWORD(lParam);
        p.y = HIWORD(lParam);
        ClientToScreen(hWnd, &p);
        TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hWnd, NULL);
        break;
    }
    case WM_CREATE: {
        repaint_bg = true;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
            
        RECT rect;
        GetClientRect(hWnd, &rect);
        // отображение игрового поля и игровых персонажей
        G->Repaint(hdc, rect.right - rect.left, rect.bottom - rect.top, repaint_bg);
        repaint_bg = false;

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        delete G;
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// окнонная процедура окна с настройками
LRESULT CALLBACK WndChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT: {
            DestroyWindow(hWnd);
            break;
        }
        case ID_CHECKBOX: {
            // -1 - никакой звек
            // 1 - первый звук
            // 2 - второй звук
            SoundPlay = !SoundPlay;
            ShowWindow(hRBttn1, SoundPlay ? SW_SHOW: SW_HIDE);
            ShowWindow(hRBttn2, SoundPlay ? 5 : 0);

            SendMessage(hRBttn1, BM_SETCHECK, BST_UNCHECKED, 0);
            SendMessage(hRBttn2, BM_SETCHECK, BST_UNCHECKED, 0);
            
            //Selected = SoundPlay ? IsDlgButtonChecked(hwnd, ID_RADIO_BTTN_F) ? 1 : IsDlgButtonChecked(hwnd, ID_RADIO_BTTN_S) ? 2 : -1 : -1;
            Selected = -1;
            break;
        }
        case ID_RADIO_BTTN_F: {
            if (HIWORD(wParam) == BN_CLICKED) { // обработка нажатия
                Selected = 1;   // первый звук
            }
            break;
        }
        case ID_RADIO_BTTN_S: {
            if (HIWORD(wParam) == BN_CLICKED) { // обработка нажатия
                Selected = 2;   // второй звук
            }
            break;
        }
        case ID_COMBOBOX: {      // обработка выбора уровня сложностиы
            if (HIWORD(wParam) == CBN_SELENDOK) {
                int nIndex = SendMessage(hCmbBx, CB_GETCURSEL, 0, 0);
                SendMessageW(hwnd, WM_COMMAND, (WPARAM)GM_WIN, nIndex);
            }
            break;
        }
        case ID_SAVENICKNAME: { // сохранение имени игрока
            if (SendMessage(hSvNChBx, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                GetWindowText(hEdt, username, MAX_LOADSTRING);  // получаем введенное имя пользователя
                if (lstrlen(username) == 0) {
                    SendMessage(hSvNChBx, BM_SETCHECK, BST_UNCHECKED, 0);   // если ничего не было введено, снимем метку
                }
                else {
                    ShowWindow(hWnd, SW_HIDE);                      // "закрытие" окна
                }
            }
            break;
        }
        case ID_USERNAME_EDIT: {
            if (HIWORD(wParam) == EN_CHANGE) {
                TCHAR buf[MAX_LOADSTRING];
                EnableWindow(hSvNChBx, GetWindowTextLength(hEdt) > 0);
                GetWindowText(hEdt, buf, MAX_LOADSTRING);
                if (lstrlen(buf) == 0) {
                    SendMessage(hSvNChBx, BM_SETCHECK, BST_UNCHECKED, 0);
                }
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    case WM_CTLCOLORSTATIC: {
        // отправляется окну когда оно должно отрисовать фон для статического элемента управления
        HWND hStatic = (HWND)lParam;        // дескриптор статического элемента
        HDC hdcStatic = (HDC)wParam;        // констекст устройства для рисования
        SetBkColor(hdcStatic, RGB(255, 255, 255));  // установка цвета фона
        // таким образом установим кисть для заливки фона статического элемента   
        return (LRESULT)GetStockObject(NULL_BRUSH); // возвращение кисти для фона
    }
    case WM_SHOWWINDOW: {
        if (wParam) { // если окно стало видимым
            EnableWindow(hwnd, false);                              // делаем родительское окно недоступным
            SendMessage(hList, LB_RESETCONTENT, 0, 0);              //  очистим listbox

            std::vector<std::wstring> game_story = loadScores();    // получим историю игр
            for (auto record : game_story) {                        // и отобразим ее
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)record.c_str());
            }
        }
        else {  // если окно сворачивается
            EnableWindow(hwnd, true);
        }
        break;
    }
    case WM_CLOSE: {    // нажатие кнопки "закрыть"
        ShowWindow(hWnd, false);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// оконная процедура для окна с уведомлением о завершении игры
LRESULT CALLBACK WndEndGameProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_SHOWWINDOW: {
        // делаем кнопку сохранить недоступной
        EnableWindow(GetDlgItem(hDlg, IDOK), false);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EDIT: {
            // проверяем, если текст в поле ввода изменился
            if (HIWORD(wParam) == EN_CHANGE) {
                HWND hwndEdit = GetDlgItem(hDlg, IDC_EDIT);
                int textLength = GetWindowTextLength(hwndEdit);
                // если текст есть, активируем кнопку "Сохранить", иначе деактивируем
                EnableWindow(GetDlgItem(hDlg, IDOK), textLength > 0);
            }
            break;
        }
        case IDOK: {
            HWND hwndEdit = GetDlgItem(hDlg, IDC_EDIT);
            GetWindowText(hwndEdit, username, MAX_LOADSTRING);
            EndDialog(hDlg, LOWORD(wParam));
            return IDOK;
        }
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return IDCANCEL;
        }
        break;
    }
    return FALSE;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
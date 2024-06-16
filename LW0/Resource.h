#define IDS_APP_TITLE			103
#define IDC_CHILD			    104

// имя файла для сохранения результатов игры
#define FILENAME "scores.txt"

// идентификаторы строк с именами звуковых файлов (LW0.rc)
#define SND_SOUND1				10
#define SND_SOUND2				11
#define SND_GAMEOVER			12

// идентификаторы для элементов в главном окне с игрой
#define ID_META_BTTN			3
#define ID_RANDTP_BTTN			5
#define ID_TABLE_STTC			6
#define ID_WAIT_BTTN			7
#define ID_WAIT_TIMER			11	// для таймера при ожидании движений

// идентификаторы для элементов в окне с настройками
#define ID_USERNAME_EDIT		13
#define ID_STATIC				14
#define ID_USERNAME_STTC		15
#define ID_LISTBOX				16
#define ID_CHECKBOX				17
#define ID_RADIO_BTTN_F			18
#define ID_RADIO_BTTN_S			19
#define ID_COMBOBOX				20
#define ID_SAVENICKNAME			21

// иденфтификаторы для битовых карт
#define IDB_BITMAP_BG			1002
#define IDB_BITMAP_CH			1003

// кодирование направления движения
// (положения курсора относительно персонажа пользователя)
// первые 3 бита горизонтальное направление - (лево (1), центр (4), право (2))
// вторые 3 бита вектикальное направление - (верх (8), центр (16), низ (32))
#define POS_NO					4 | 32
#define POS_LT					1 | 8
#define POS_LB					1 | 16
#define POS_LL					1 | 32
#define POS_TT					4 | 8
#define POS_BB					4 | 16
#define POS_RT					2 | 8
#define POS_RR					2 | 32
#define POS_RB					2 | 16

// идентификаторы для курсоров
#define IDC_CURNO				12001
#define IDC_CURLT				12002
#define IDC_CURLB				12003
#define IDC_CURLL				12004
#define IDC_CURTT				12005
#define IDC_CURBB				12006
#define IDC_CURRT				12007
#define IDC_CURRR				12008
#define IDC_CURRB				12009
#define IDC_CURPL				12010

// идентификаторы для отслеживания состояния игры
// (окну с игрой отправляется сообщение в зависимости от состояния игры)
#define GM_LOSE					0
#define GM_WIN					1
#define GM_CONTINUE				2

// идентификаторы для окон
#define IDM_DIALOG				102	// идентификатор для открытия окна с вводом имени
#define IDD_DIALOG				107 // идентификатор для окна с вводом имени
#define IDM_ABOUT				104 // идентификатор для открытия окна "о приложении"
#define IDD_ABOUTBOX			103	// идентификатор для окна "о приложении"
#define IDM_SETTINGS		    108	// идентификатор для отображения окна с настройками

#define IDM_METAFILE			1001
#define IDR_MAINFRAME			128
#define IDM_HELP				106
#define IDM_EXIT				105
#define IDI_LW0					107
#define IDI_SMALL				108
#define IDC_EDIT				110
#define IDC_LW0					109
#define IDC_MYICON				2
#ifndef IDC_STATIC
#define IDC_STATIC				-1
#endif
// Следующие стандартные значения для новых объектов
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NO_MFC					130
#define _APS_NEXT_RESOURCE_VALUE	129
#define _APS_NEXT_COMMAND_VALUE		32771
#define _APS_NEXT_CONTROL_VALUE		1000
#define _APS_NEXT_SYMED_VALUE		110
#endif
#endif

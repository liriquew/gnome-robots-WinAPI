// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include <vector>
#include "targetver.h"
#include <cstdlib>						// Для функций rand() и srand()
#include <algorithm>					// Для функции std::find()
#include <fstream>
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include "windows.h"
#include "wingdi.h"
#include "mmsystem.h"					// для функции PlaySound()
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
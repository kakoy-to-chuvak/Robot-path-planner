#ifndef __FILESAVE_H__
#define __FILESAVE_H__

#include <SDL3/SDL_dialog.h>
#include <stdlib.h>
#include <string.h>
#include "path.h"
#include "logs.h"


typedef enum FILESAVE_FORMAT {
        SAVE_FORMAT_JSON = 0,
        SAVE_FORMAT_TXT = 1,
        SAVE_FORMAT_CSV = 2,
        SAVE_FORMAT_UNDEFINED = 3,
} FILESAVE_FORMAT;


void SavePoints(PArray* _Points, const char *_Filename, FILESAVE_FORMAT _Save_format);
void OpenFIleDialog(SDL_Window *_Window, const char *_Default_location, PArray *_Points);








#endif //__FILESAVE_H__
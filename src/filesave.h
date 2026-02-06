#ifndef __FILESAVE_H__
#define __FILESAVE_H__

#include <SDL3/SDL_dialog.h>

#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "logs.h"
#include "parametrs.h"

typedef struct FileSaveArgs {
        PArray *points;
        Parametrs *parametrs;

} FileSaveArgs;

void SavePoints(PArray *_Points);
void ShowSaveFIleDialog(SDL_Window *_Window, const char *_Default_location, FileSaveArgs *_Args);

void LoadPoints(PArray *_Points, Parametrs *_Parametrs);
void ShowOpenFIleDialog(SDL_Window *_Window, const char *_Default_location, FileSaveArgs *_Args);








#endif //__FILESAVE_H__
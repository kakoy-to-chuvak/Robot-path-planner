#ifndef __PATH_H__
#define __PATH_H__


#include <stdint.h>

#include <SDL3/SDL.h>

#include "include.h"
#include "vectors.h"
#include "drawing.h"
#include "label.h"




typedef enum PState {
        PSTATE_UNDEFINED = -1,
        PSTATE_NONE_STATE = 0,
        PSTATE_UNDER_MOUSE = 1,
        PSTATE_SELECTED = 2,
} PState;

typedef struct Point {
        SDL_FPoint cords;
        PState state;
        PState line_state;
        struct Point *next;
        struct Point *prev;
} Point;

typedef enum FILESAVE_FORMAT {
        FILE_FORMAT_JSON = 0,
        FILE_FORMAT_PTS = 1,
        FILE_FORMAT_CSV = 2,
        FILE_FORMAT_UNDEFINED = 3,
} FILESAVE_FORMAT;

typedef struct PArray {
        bool changed;
        Point *points;
        Point *selected_point;
        Point *selected_line;
        char file_name[MAX_PATH];
        FILESAVE_FORMAT format;
} PArray;



void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, SDL_FRect texture_box);
bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, SDL_FRect texture_box, bool mouse_pressed, bool prev_mouse_state, bool shift_pressed, bool ctrl_pressed, bool alt_pressed);
void AddPoint(PArray *points, SDL_FPoint cords, Point *line);
void AddPoint_tostart(PArray *points, SDL_FPoint cords);
void DelPoint(PArray *points, Point *point);

void FreePoints(PArray *_Points);



#endif // __PATH_H__
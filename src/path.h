#ifndef __PATH_H__
#define __PATH_H__


#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>

#include "parametrs.h"
#include "vectors.h"
#include "drawing.h"
#include "label.h"




typedef enum PState {
        PSTATE_UNDEFINED = -1,
        PSTATE_NONE_STATE = 0,
        PSTATE_UNDER_MOUSE = 1,
        PSTATE_SELECTED = 2,
        PSTATE_LINE_UNDER_MOUSE = 3,
        PSTATE_LINE_SELECTED = 4,
        PSTATE_VECTOR_UNDER_MOUSE = 5,
        PSTATE_VECTOR_SELECTED = 6,
} PState;

typedef struct Point {
        SDL_FPoint cords;
        float angle;
        PState state;
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
        char file_name[MAX_PATH];
        FILESAVE_FORMAT format;
} PArray;



void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, Parametrs *_Parametrs);

bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parametrs);

void AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line, Parametrs *_Parametrs);
void AddPoint_tostart(PArray *points, SDL_FPoint cords, float angle, Parametrs *_Parametrs);
void DelPoint(PArray *points, Point *point);

void FreePoints(PArray *_Points);



#endif // __PATH_H__
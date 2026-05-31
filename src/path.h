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


#define NULL_USER_FIELD ((UserField){NULL,NULL,NULL})


typedef enum ACTION_TYPE {
        DEL_POINT,
        ADD_POINT,
        ADD_POINT_TO_START,
        MOVE_POINT,
} ACTION_TYPE;

typedef struct ACTION {
        ACTION_TYPE type;
        uint32_t id;
        uint32_t prev_id;

        SDL_FPoint start_cords;
        float start_angle;
        
        SDL_FPoint res_cords;
        float res_angle;

        struct ACTION *next;
        struct ACTION *prev;
} ACTION;

// State of point
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

// custom user json field
typedef struct UserField {
        char *key;
        char *value;
        struct UserField *next;
} UserField;

// point struct
typedef struct Point {
        SDL_FPoint cords;
        float angle;
        PState state;
        UserField *user_fields;
        uint32_t id;

        struct Point *next;
        struct Point *prev;
} Point;

// array of points
typedef struct PArray {
        bool changed;
        int count;
        Point *points;
        Point *selected_point;
        char file_name[MAX_PATH];
} PArray;


SDL_FPoint BoxCordsToWindow(SDL_FPoint cords, Parametrs *_Parametrs);
SDL_FPoint WindowCordsToBox(SDL_FPoint cords, Parametrs *_Parametrs);

void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, Parametrs *_Parametrs);

// Check events and move points
bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parametrs);

// adding / removing points
Point *AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line, UserField *_Fields, Parametrs *_Parametrs);
Point *AddPoint_tostart(PArray *points, SDL_FPoint cords, float angle, UserField *_Fields, Parametrs *_Parametrs);
Point *AddPointAfter(PArray *_Points, uint32_t _Source_id, SDL_FPoint cords, float angle, UserField *_Fields, Parametrs *_Parametrs);
void DelPoint(PArray *points, Point *point);

// Freeing points in PArray
void FreeUserFields(UserField *_Field);
void FreePoint(Point *_Point) ;
void FreePoints(PArray *_Points);

bool PathUndo(PArray *_Points);
bool PathRedo(PArray *_Points);
bool PathAddAction(ACTION_TYPE _Type, Point *_Point, SDL_FPoint _Start_cord, double _Start_ang);
void PathClearActions();
void PathFreeAction(ACTION *_Action);



#endif // __PATH_H__
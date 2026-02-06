#ifndef PARAMETRS_H
#define PARAMETRS_H


#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <SDL3/SDL.h>

#include "crossplatform.h"

#define SDL_INIT_FLAGS (SDL_INIT_VIDEO)
#define APP_TITLE "Планировщик маршрута"

#define POINT_CORDS_FONT  "Hasklig-Black.ttf"
#define MENU_TEXT_FONT    "Hasklig-Regular.ttf"

#define TPS 60

#define TEXT_SIZE (35)

#define TEXT_COLOR_Black ((SDL_Color){0, 0, 0, 255})
#define TEXT_COLOR_BrightWhite ((SDL_Color){255, 255, 255, 255})
#define TEXT_COLOR_White ((SDL_Color){222, 222, 222, 255})
#define TEXT_COLOR_Grey ((SDL_Color){128, 128, 128, 255})

#define TEXT_PARAMS ((LABEL_PARAMS){TEXT_COLOR_White, 3})


#define MENU_BG ((SDL_Color){31, 31, 31, 255})
#define MENU_BORDER_COLOR ((SDL_Color){255, 0, 0, 120})
#define MENU_TRIGGER_COLOR ((SDL_Color){0, 123, 208, 255})


typedef struct _Parametrs_struct_ {

        bool lmb_pressed;
        bool rmb_pressed;

        bool prev_lmb_state;
        bool prev_rmb_state;

        bool shift_pressed;
        bool ctrl_pressed;
        bool alt_pressed;

        int window_w;
        int window_h;

        int monitor_h;
        int monitor_w;

        SDL_FRect texture_box;
        float box_width;
        float box_height;

        int point_radius;
        int point_diametr;
        int line_width;
        int line_arrow_base;

        int dir_vector_legth;
        int dir_vector_width;
        int dir_vector_arrow_base;

        float fixed_point_radius;
        float fixed_line_width;

        float fixed_dir_vector_legth;
        float fixed_dir_vector_width;

} Parametrs;


void ParametrsInit(Parametrs *_Parametrs, SDL_Window *_Window);
void ParametrsFixValues(Parametrs *_Parametrs, SDL_Window *_Window);

void ResizeBgTexture(Parametrs *_Parametrs);


#endif  // PARAMETRS_H
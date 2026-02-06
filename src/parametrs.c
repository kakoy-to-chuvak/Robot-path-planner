#include "parametrs.h"


#define POINT_MONITOR_RADIUS 0.008f

#define LINE_WIDTH 6
#define LINE_ARROW_BASE 20

#define DIR_VECTOR_WIDTH 4
#define DIR_VECTOR_ARROW_SIZE 10

#define BOX_HEIGHT 2.0f
#define BOX_WIDTH  3.0f



void ParametrsInit(Parametrs *_Parametrs, SDL_Window *_Window) {
        _Parametrs->lmb_pressed = 0;
        _Parametrs->rmb_pressed = 0;

        _Parametrs->prev_lmb_state = 0;
        _Parametrs->prev_rmb_state = 0;
        
        _Parametrs->shift_pressed = 0;
        _Parametrs->ctrl_pressed = 0;
        _Parametrs->alt_pressed = 0;

        _Parametrs->window_h = 0;
        _Parametrs->window_w = 0;

        _Parametrs->monitor_h = 0;
        _Parametrs->monitor_w = 0;

        _Parametrs->texture_box = (SDL_FRect) {
                0, 0, 0, 0
        };

        _Parametrs->box_width = BOX_WIDTH;
        _Parametrs->box_height = BOX_HEIGHT;

        ParametrsFixValues(_Parametrs, _Window);
}


void ParametrsFixValues(Parametrs *_Parametrs, SDL_Window *_Window) {
        SDL_DisplayID id = SDL_GetDisplayForWindow(_Window);

        const SDL_DisplayMode *dm = SDL_GetDesktopDisplayMode(id);
        _Parametrs->monitor_w = dm->w;
        _Parametrs->monitor_h = dm->h;

        SDL_GetWindowSize(_Window, &_Parametrs->window_w, &_Parametrs->window_h);

        ResizeBgTexture(_Parametrs);
        
        _Parametrs->point_radius = sqrt( _Parametrs->monitor_w * _Parametrs->monitor_w + _Parametrs->monitor_h * _Parametrs->monitor_h ) * POINT_MONITOR_RADIUS;
        _Parametrs->point_diametr = _Parametrs->point_radius * 2;
        _Parametrs->line_width = _Parametrs->point_radius / 3;
        _Parametrs->line_arrow_base = _Parametrs->point_radius;
        
        _Parametrs->dir_vector_legth = _Parametrs->point_radius * 3;
        _Parametrs->dir_vector_width = _Parametrs->point_radius / 4;
        _Parametrs->dir_vector_arrow_base = _Parametrs->point_radius / 2;
        
        float k;
        if ( _Parametrs->texture_box.h && _Parametrs->texture_box.w ) {
                k = _Parametrs->box_width / _Parametrs->texture_box.w;
        } else {
                k = 0.0f;
        }
        
        _Parametrs->fixed_point_radius = _Parametrs->point_radius * k;
        _Parametrs->fixed_line_width = _Parametrs->line_width * k;

        _Parametrs->fixed_dir_vector_legth = _Parametrs->dir_vector_legth * k;
        _Parametrs->fixed_dir_vector_width = _Parametrs->dir_vector_width * k;
}


void ResizeBgTexture(Parametrs *_Parametrs) {
        float k = _Parametrs->box_width / _Parametrs->box_height;
        
        // resize background texture
        if ( ((float)_Parametrs->window_w) / ((float)_Parametrs->window_h) > k ) {
                _Parametrs->texture_box.w = _Parametrs->window_h * k;
                _Parametrs->texture_box.h = _Parametrs->window_h;
                _Parametrs->texture_box.y = 0;
                _Parametrs->texture_box.x = ( _Parametrs->window_w - _Parametrs->texture_box.w ) / 2;
        } else {
                _Parametrs->texture_box.h = _Parametrs->window_w / k;
                _Parametrs->texture_box.w = _Parametrs->window_w;
                _Parametrs->texture_box.x = 0;
                _Parametrs->texture_box.y = ( _Parametrs->window_h - _Parametrs->texture_box.h ) / 2;
        }
}
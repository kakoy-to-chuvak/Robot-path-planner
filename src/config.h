#ifndef CONFIG_H
#define CONFIG_H


#include <SDL3/SDL.h>
#include <stdlib.h>

#include "config_parser.h"



struct _struct_app_config {
        char title[32];
        int width;
        int height;
        int tps;
        int fps;
};

struct _struct_menu_config {
        SDL_Color background;
        SDL_Color trigger_color;
        SDL_Color border_color;
        
        SDL_Color text_color;
        SDL_Color text_inactive_color;
        char font[_MAX_PATH];
        int text_size;
};


struct _struct_points_config {
        int radius;
        int diametr;

        int line_radius;
        int arrow_base;
};

struct _struct_main_config {
        char *font;
        int text_size;
        SDL_Color text_color;
        SDL_Color border_color;
};


typedef struct _struct_CONFIG {
        struct _struct_app_config app;
        struct _struct_menu_config menu;
        struct _struct_points_config points;
        struct _struct_main_config main;
} CONFIG;








#endif // CONFIG_H
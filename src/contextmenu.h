#ifndef __MENU_H__
#define __MENU_H__


#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_pixels.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdarg.h>

#include "logs.h"
#include "label.h"
#include "drawing.h"


typedef struct MENU_BUTTON {
        int id;

        bool hide;
        bool active;
        bool triggered;

        LABEL *label;
        void* (*function)(void *menu, void *args);

        struct MENU_BUTTON *next;
        struct MENU_BUTTON *prev;
} MENU_BUTTON; 


typedef struct CONTEXTMENU {
        SDL_Renderer *renderer;

        SDL_PixelFormat pixel_format;
        SDL_Texture *menu_texture;
        SDL_Texture *button_texture;
        SDL_Texture *trigger_texture;

        bool active;

        int32_t x;
        int32_t y;
        int32_t w;
        int32_t h;
        
        int32_t border_width;
        float border_radius;
        
        SDL_Color border_color;
        SDL_Color bg_color;

        int32_t buttons_count;
        MENU_BUTTON *buttons;
        
        SDL_Color button_bg_color;
        SDL_Color trigger_color;

        int32_t text_indent_x;
        int32_t text_indent_y;

        int32_t button_w;
        int32_t button_h;
        int32_t button_indent_x;
        int32_t button_indent_y;
        float button_radius;
} CONTEXTMENU;


CONTEXTMENU *ContextMenu_New( SDL_Renderer *renderer, SDL_PixelFormat pixel_format,
                SDL_Color background, float border_radius,
                int32_t border, SDL_Color border_color );
void ContextMenu_Move(CONTEXTMENU *menu, float x, float y, float window_w, float window_h);
bool ContextMenu_MouseOut(CONTEXTMENU *menu, int32_t mouse_x, int32_t mouse_y);
bool ContextMenu_CheckUpdate(CONTEXTMENU *menu, float mouse_x, float mouse_y, bool click, void *function_args);
void ContextMenu_Free(CONTEXTMENU *menu);


void ContextMenu_Render(CONTEXTMENU *menu);


void ContextMenu_SetupButtons( CONTEXTMENU *menu, float radius,
                        int32_t width, int32_t height,
                        SDL_Color background, SDL_Color trigger_color,
                        int32_t indent_w, int32_t indent_h,
                        int32_t text_indent_w, int32_t text_indent_h);
                


MENU_BUTTON *ContextMenu_SetButton(    CONTEXTMENU *menu, 
                                int id,
                                LABEL *label,
                                bool hide,
                                bool active,
                                void* (*function)(void*, void*) );

MENU_BUTTON *ContextMenu_GetButton(CONTEXTMENU *menu, int id);
bool ContextMenu_DelButton(CONTEXTMENU *menu, MENU_BUTTON *button);
void ContextMenu_HideButton(CONTEXTMENU *menu, MENU_BUTTON *button, bool hide);


SDL_Texture *ContextMenu_CreateMenuTexture(CONTEXTMENU *menu);
bool ContextMenu_CreateButtonTexture(CONTEXTMENU *menu);


#endif // __MENU_H__
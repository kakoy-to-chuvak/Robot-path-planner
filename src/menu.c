#include "menu.h"


float fix_cord(float x, float w, float app_x) {
    if (x + w > app_x) 
        return app_x - w;
    if (x < 0) 
        return 0;
    return x;
}


// ==== Main functions ====
MENU *Menu_New( SDL_Renderer *renderer, SDL_PixelFormat pixel_format,
                SDL_Color background, float border_radius,
                int32_t border, SDL_Color border_color ) 
{

        MENU *menu = malloc(sizeof(MENU));
        if ( NULL == menu )
                return NULL;

        menu->h = 0;
        menu->w = 0;
        menu->x = 0;
        menu->y = 0;
        
        menu->bg_color = background;
        menu->border_color = border_color;
        menu->border_width = border;
        menu->border_radius = border_radius;

        menu->renderer = renderer;
        menu->active = 0;
        menu->buttons = NULL;
        menu->buttons_count = 0;

        menu->button_h = 0;
        menu->button_w = 0;

        menu->button_indent_y = 0;
        menu->button_indent_x = 0;

        menu->text_indent_y = 0;
        menu->text_indent_x = 0;

        menu->button_radius = 0;

        menu->pixel_format = pixel_format;
        Menu_CreateMenuTexture(menu);
        menu->button_texture = NULL;
        menu->trigger_texture = NULL;

        return menu;
}



void Menu_Move(MENU *menu, float x, float y, float window_w, float window_h) {
        if ( menu == NULL ) 
                return;

        menu->x = fix_cord(x, menu->border_width + menu->w, window_w);
        menu->y = fix_cord(y, menu->border_width + menu->h, window_h);
}



bool Menu_MouseOut(MENU *menu, int32_t mouse_x, int32_t mouse_y) {
        return  mouse_x < menu->x - menu->border_width || 
                mouse_y < menu->y - menu->border_width || 
                mouse_x > menu->x + menu->w + menu->border_width || 
                mouse_y > menu->y + menu->h + menu->border_width;
}



bool Menu_CheckUpdate(MENU *menu, float mouse_x, float mouse_y, bool click, void *function_args) {
        if ( NULL == menu || menu->active == 0 ) 
                return 0;
        
        
        bool res = 0;

        bool mouse_out_menu = Menu_MouseOut(menu, mouse_x, mouse_y);
        if ( mouse_out_menu && click ) {
                menu->active = 0;
                res = 1;
        }

        int32_t now_y = menu->y + menu->button_indent_y;
        int32_t button_x = menu->x + menu->button_indent_x;
        int32_t button_h = menu->button_h;
        int32_t button_w = menu->button_w;

        MENU_BUTTON *now = menu->buttons;
        while ( now ) {
                if ( mouse_out_menu || now->hide == 1 ) {
                        if ( now->triggered )
                                res = 1;
                        now->triggered = 0;
                        now = now->next;
                        continue;
                }

                if ( now->active == 0 ) {
                        now->triggered = 0;
                        goto next;
                }       

                if ( mouse_x > button_x && mouse_x < button_x + button_w &&
                     mouse_y > now_y && mouse_y < now_y + button_h ) {
                        if ( now->triggered == 0 ) {
                                res = 1;
                                now->triggered = 1;
                        }

                        if ( click ) {
                                res = 1;
                                if ( now->function ) {
                                        now->function(menu, function_args);
                                }
                                menu->active = 0;
                        }
                } else {
                        if ( now->triggered == 1 ) {
                                res = 1;
                                now->triggered = 0;
                        }
                }

                next:
                now = now->next;
                now_y += menu->button_h;
        }

        return res;
}



void Menu_Free(MENU *menu) {
        if ( NULL == menu ) 
                return;

        MENU_BUTTON *now = menu->buttons;
        while ( now ) {
                MENU_BUTTON *tmp = now->next;
                free(now);
                now = tmp;
        }

        free(menu);
}


//  ==== Render function ====
void Menu_Render(MENU *menu) {
        if ( NULL == menu || menu->active == 0) 
                return;

        float x = menu->x;
        float y = menu->y;

        SDL_FRect rect = (SDL_FRect){
                x, 
                y, 
                menu->w + 2 * menu->border_width, 
                menu->h  + 2 * menu->border_width
        };

        if ( menu->menu_texture )
                SDL_RenderTexture(menu->renderer, menu->menu_texture, NULL, &rect);

        MENU_BUTTON *now = menu->buttons;
        rect.x += menu->button_indent_x + menu->border_width;
        rect.y += menu->button_indent_y + menu->border_width;
        rect.w = menu->button_w;
        rect.h = menu->button_h;

        SDL_FRect label_rect = (SDL_FRect){
                rect.x + menu->text_indent_x,
                rect.y + menu->text_indent_y,
                0,
                menu->button_h - 2 * menu->text_indent_y
        };

        while ( now ) {
                if ( now->hide )
                        goto next;
                
                SDL_RenderTexture(menu->renderer, now->triggered ? menu->trigger_texture : menu->button_texture, NULL, &rect);

                if ( now->label ) {
                        label_rect.w = label_rect.h * now->label->rect.w / now->label->rect.h;
                        Label_Draw(now->label, NULL, &label_rect);
                } 

                rect.y += menu->button_h;
                label_rect.y += menu->button_h;

                next:
                now = now->next;
        }
}



// ==== Menu buttons ====
void Menu_SetupButtons( MENU *menu, float radius,
                        int32_t width, int32_t height,
                        SDL_Color background, SDL_Color trigger_color,
                        int32_t indent_w, int32_t indent_h,
                        int32_t text_indent_w, int32_t text_indent_h) 
{
        if ( menu == NULL )
                return;

        menu->button_bg_color = background;
        menu->trigger_color = trigger_color;
        
        menu->w = width + 2 * indent_w;
        menu->h = menu->h - 2 * menu->button_indent_y + menu->buttons_count * ( height - menu->button_h )+ 2 * indent_h;

        menu->button_h = height;
        menu->button_w = width;


        menu->button_indent_y = indent_h;
        menu->button_indent_x = indent_w;

        menu->text_indent_y = text_indent_h;
        menu->text_indent_x = text_indent_w;

        menu->button_radius = radius;

        Menu_CreateMenuTexture(menu);
        Menu_CreateButtonTexture(menu);

}



MENU_BUTTON *Menu_SetButton(    MENU *menu, 
                                int id,
                                LABEL *label,
                                bool hide,
                                bool active,
                                void* (*function)(void*, void*) 
                        )
{       
        MENU_BUTTON *button = Menu_GetButton(menu, id);

        if ( button == NULL) {
                button = malloc(sizeof(MENU_BUTTON));
                if ( button == NULL)
                        return NULL;
                        
                if ( menu->buttons ) 
                        menu->buttons->prev = button;
                button->prev = NULL;
                button->next = menu->buttons;
                menu->buttons = button;

                button->id = id;
                menu->buttons_count++;
                menu->h += menu->button_h;
        }
        
        button->triggered = 0;
        button->hide = hide;
        button->active = active;

        button->function = function;
        button->label = label;  

        Menu_CreateMenuTexture(menu);

        return button;
}



MENU_BUTTON *Menu_GetButton(MENU *menu, int id) {
        MENU_BUTTON *now = menu->buttons;
        while ( now ) {
                if ( now->id == id )
                        return now;
                now = now->next;
        }

        return NULL;
}



bool Menu_DelButton(MENU *menu, MENU_BUTTON *button) {
        if ( NULL == menu || NULL == button ) 
                return 0;
        
        if ( button->prev )
                button->prev->next = button->next;
        else 
                menu->buttons = button->next;

        if ( button->next )
                button->next->prev = button->prev;

        menu->buttons_count--;
        menu->h -= menu->button_h;

        free(button);

        return 1;
}



void Menu_HideButton(MENU *menu, MENU_BUTTON *button, bool hide) {
        if ( button == NULL )
                return;

        if ( button->hide == 0 && hide == 1 ) {
                menu->buttons_count--;
                menu->h -= menu->button_h;
        } else if ( button->hide == 1 && hide == 0 ) {
                menu->buttons_count++;
                menu->h += menu->button_h;
        }

        button->hide = hide;
}




// surface
void __Surface_DrawStraightLine(SDL_Surface *surf, int32_t x, int32_t y, int32_t len, Uint32 color) {
        SDL_Rect rect = (SDL_Rect){
                x,
                y,
                len,
                1
        };

        SDL_FillSurfaceRect(surf, &rect, color);
}



void __Menu_FillSurfaceBorder(SDL_Surface *surf, Uint32 w, Uint32 h, int32_t border_width, int32_t radius, Uint32 color) {
        SDL_Rect now_rect = {
                0,
                radius,
                border_width,
                h - 2 * radius
        };

        SDL_FillSurfaceRect(surf, &now_rect, color);
        
        now_rect = (SDL_Rect){
                radius - 1,
                0,
                w - 2 * radius + 2,
                border_width
        };

        SDL_FillSurfaceRect(surf, &now_rect, color);

        now_rect = (SDL_Rect){
                radius - 1,
                h - border_width,
                w - 2 * radius + 2,
                border_width
        };

        SDL_FillSurfaceRect(surf, &now_rect, color);

        now_rect = (SDL_Rect){
                w - border_width,
                radius,
                border_width,
                h - 2 * radius
        };

        SDL_FillSurfaceRect(surf, &now_rect, color);

        // Circle midpoint algoritm
        if ( radius )
                radius--;

        float x_left = radius;
        float y_up = radius;
        float x_right = w - radius - 1;
        float y_bottom = h - radius - 1;

        float x = 0;
        float y = radius;
        float p = 1 - radius;
        
        __Surface_DrawStraightLine(surf, x_left - y, y_up - x, y, color);
        __Surface_DrawStraightLine(surf, x_right + 1, y_up - x, y, color);
        __Surface_DrawStraightLine(surf, x_left - y, y_bottom + x, y, color);
        __Surface_DrawStraightLine(surf, x_right + 1, y_bottom + x, y, color);
        __Surface_DrawStraightLine(surf, x_left - x, y_up - y, x, color);
        __Surface_DrawStraightLine(surf, x_right + 1, y_up - y, x, color);
        __Surface_DrawStraightLine(surf, x_left - x, y_bottom + y, x, color);
        __Surface_DrawStraightLine(surf, x_right + 1, y_bottom + y, x, color);
                
        while (x < y)
        {
        	if ( p < 0 ) {
                        p = p + 2 * x + 1;
                } else {
                        p = p + 2 * ( x - y ) + 1;
                        y--;
                }
                
                x++;

                __Surface_DrawStraightLine(surf, x_left - y, y_up - x, y, color);
                __Surface_DrawStraightLine(surf, x_right + 1, y_up - x, y, color);
                __Surface_DrawStraightLine(surf, x_left - y, y_bottom + x, y, color);
                __Surface_DrawStraightLine(surf, x_right + 1, y_bottom + x, y, color);
                __Surface_DrawStraightLine(surf, x_left - x, y_up - y, x, color);
                __Surface_DrawStraightLine(surf, x_right + 1, y_up - y, x, color);
                __Surface_DrawStraightLine(surf, x_left - x, y_bottom + y, x, color);
                __Surface_DrawStraightLine(surf, x_right + 1, y_bottom + y, x, color);   
        }
}



void __Menu_SurfaceFillRect(SDL_Surface *surf, SDL_Rect rect, int32_t radius, Uint32 color) {
        SDL_Rect now_rect = {
                rect.x,
                rect.y + radius,
                rect.w,
                rect.h - 2 * radius
        };

        SDL_FillSurfaceRect(surf, &now_rect, color);

        float x_l = rect.x + radius;
        float y_u = rect.y + radius;
        float x_r = rect.w -  2 * radius;
        float y_b = rect.y + rect.h - radius - 1;

        float x = 0;
        float y = radius;
        float p = 1 - radius;

        __Surface_DrawStraightLine(surf, x_l - y, y_u - x, x_r + 2 * y, color);
        __Surface_DrawStraightLine(surf, x_l - y, y_b + x, x_r + 2 * y, color);
        __Surface_DrawStraightLine(surf, x_l - x, y_u - y, x_r + 2 * x, color);
        __Surface_DrawStraightLine(surf, x_l - x, y_b + y, x_r + 2 * x, color);
                
        while (x < y)
        {       
                if ( p < 0 ) {
                        p = p + 2 * x + 1;
                } else {
                        p = p + 2 * ( x - y ) + 1;
                        y--;
                }

                x++;
 
                __Surface_DrawStraightLine(surf, x_l - y, y_u - x, x_r + 2 * y, color);
                __Surface_DrawStraightLine(surf, x_l - y, y_b + x, x_r + 2 * y, color);
                __Surface_DrawStraightLine(surf, x_l - x, y_u - y, x_r + 2 * x, color);
                __Surface_DrawStraightLine(surf, x_l - x, y_b + y, x_r + 2 * x, color);
        }
}



SDL_Texture *Menu_CreateMenuTexture(MENU *menu) {
        if ( menu == NULL ) 
                return NULL;
      
        SDL_Surface *surf = SDL_CreateSurface(menu->w + 2 * menu->border_width, menu->h + 2 * menu->border_width, menu->pixel_format);
        if ( surf == NULL )
                return NULL;

        Uint32 color = SDL_MapRGBA(SDL_GetPixelFormatDetails(menu->pixel_format), NULL, menu->border_color.r, menu->border_color.g, menu->border_color.b, menu->border_color.a);
        
        if ( menu->border_width )
                __Menu_FillSurfaceBorder(surf, menu->w + 2 * menu->border_width, menu->h + 2 * menu->border_width, menu->border_width, menu->border_radius, color);
        
        SDL_Rect tmp = (SDL_Rect){
                menu->border_width,
                menu->border_width,
                menu->w,
                menu->h
        };

        color = SDL_MapRGBA(SDL_GetPixelFormatDetails(menu->pixel_format), NULL, menu->bg_color.r, menu->bg_color.g, menu->bg_color.b, menu->bg_color.a);

        __Menu_SurfaceFillRect(surf, tmp, menu->border_radius, color);

        menu->menu_texture = SDL_CreateTextureFromSurface(menu->renderer, surf);

        SDL_DestroySurface(surf);

        return menu->menu_texture;
}


bool Menu_CreateButtonTexture(MENU *menu) {
        if ( menu == NULL )
                return NULL;

        SDL_Surface *surf = SDL_CreateSurface(menu->button_w, menu->button_h, menu->pixel_format);
        if ( surf == NULL )
                return NULL;

        SDL_Rect rect = (SDL_Rect){
                0,
                0,
                menu->button_w,
                menu->button_h
        };

        // button texture
        Uint32 color = SDL_MapRGBA( SDL_GetPixelFormatDetails(menu->pixel_format), NULL, 
                                    menu->button_bg_color.r, menu->button_bg_color.g, 
                                    menu->button_bg_color.b, menu->button_bg_color.a);

        __Menu_SurfaceFillRect(surf, rect, menu->button_radius, color);

        menu->button_texture = SDL_CreateTextureFromSurface(menu->renderer, surf);

        // trigger texture
        color = SDL_MapRGBA( SDL_GetPixelFormatDetails(menu->pixel_format), NULL, 
                             menu->trigger_color.r, menu->trigger_color.g, 
                             menu->trigger_color.b, menu->trigger_color.a);

        __Menu_SurfaceFillRect(surf, rect, menu->button_radius, color);

        menu->trigger_texture = SDL_CreateTextureFromSurface(menu->renderer, surf);

        SDL_DestroySurface(surf);

        return menu->button_texture && menu->trigger_texture;
}
#include "contextmenu.h"


float fix_cord(float x, float w, float app_x) {
    if (x + w > app_x) 
        return app_x - w;
    if (x < 0) 
        return 0;
    return x;
}


// ==== Main functions ====
CONTEXTMENU *ContextMenu_New( SDL_Renderer *renderer, SDL_PixelFormat pixel_format,
                SDL_Color background, float border_radius,
                int32_t border, SDL_Color border_color ) 
{

        CONTEXTMENU *contextmenu = malloc(sizeof(CONTEXTMENU));
        if ( NULL == contextmenu )
                return NULL;

        contextmenu->h = 0;
        contextmenu->w = 0;
        contextmenu->x = 0;
        contextmenu->y = 0;
        
        contextmenu->bg_color = background;
        contextmenu->border_color = border_color;
        contextmenu->border_width = border;
        contextmenu->border_radius = border_radius;

        contextmenu->renderer = renderer;
        contextmenu->active = 0;
        contextmenu->buttons = NULL;
        contextmenu->buttons_count = 0;

        contextmenu->button_h = 0;
        contextmenu->button_w = 0;

        contextmenu->button_indent_y = 0;
        contextmenu->button_indent_x = 0;

        contextmenu->text_indent_y = 0;
        contextmenu->text_indent_x = 0;

        contextmenu->button_radius = 0;

        contextmenu->pixel_format = pixel_format;
        ContextMenu_CreateMenuTexture(contextmenu);
        contextmenu->button_texture = NULL;
        contextmenu->trigger_texture = NULL;

        return contextmenu;
}



void ContextMenu_Move(CONTEXTMENU *contextmenu, float x, float y, float window_w, float window_h) {
        if ( contextmenu == NULL ) 
                return;

        contextmenu->x = fix_cord(x, contextmenu->border_width + contextmenu->w, window_w);
        contextmenu->y = fix_cord(y, contextmenu->border_width + contextmenu->h, window_h);
}



bool ContextMenu_MouseOut(CONTEXTMENU *contextmenu, int32_t mouse_x, int32_t mouse_y) {
        return  mouse_x < contextmenu->x - contextmenu->border_width || 
                mouse_y < contextmenu->y - contextmenu->border_width || 
                mouse_x > contextmenu->x + contextmenu->w + contextmenu->border_width || 
                mouse_y > contextmenu->y + contextmenu->h + contextmenu->border_width;
}



bool ContextMenu_CheckUpdate(CONTEXTMENU *contextmenu, float mouse_x, float mouse_y, bool click, void *function_args) {
        if ( NULL == contextmenu || contextmenu->active == 0 ) 
                return 0;
        
        
        bool res = 0;

        bool mouse_out_menu = ContextMenu_MouseOut(contextmenu, mouse_x, mouse_y);
        if ( mouse_out_menu && click ) {
                contextmenu->active = 0;
                res = 1;
        }

        int32_t now_y = contextmenu->y + contextmenu->button_indent_y;
        int32_t button_x = contextmenu->x + contextmenu->button_indent_x;
        int32_t button_h = contextmenu->button_h;
        int32_t button_w = contextmenu->button_w;

        MENU_BUTTON *now = contextmenu->buttons;
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
                                        now->function(contextmenu, function_args);
                                }
                                contextmenu->active = 0;
                        }
                } else {
                        if ( now->triggered == 1 ) {
                                res = 1;
                                now->triggered = 0;
                        }
                }

                next:
                now = now->next;
                now_y += contextmenu->button_h;
        }

        return res;
}



void ContextMenu_Free(CONTEXTMENU *contextmenu) {
        if ( NULL == contextmenu ) 
                return;

        MENU_BUTTON *now = contextmenu->buttons;
        while ( now ) {
                MENU_BUTTON *tmp = now->next;
                free(now);
                now = tmp;
        }

        free(contextmenu);
}


//  ==== Render function ====
void ContextMenu_Render(CONTEXTMENU *contextmenu) {
        if ( NULL == contextmenu || contextmenu->active == 0) 
                return;

        float x = contextmenu->x;
        float y = contextmenu->y;

        SDL_FRect rect = (SDL_FRect){
                x, 
                y, 
                contextmenu->w + 2 * contextmenu->border_width, 
                contextmenu->h  + 2 * contextmenu->border_width
        };

        if ( contextmenu->menu_texture )
                SDL_RenderTexture(contextmenu->renderer, contextmenu->menu_texture, NULL, &rect);

        MENU_BUTTON *now = contextmenu->buttons;
        rect.x += contextmenu->button_indent_x + contextmenu->border_width;
        rect.y += contextmenu->button_indent_y + contextmenu->border_width;
        rect.w = contextmenu->button_w;
        rect.h = contextmenu->button_h;

        SDL_FRect label_rect = (SDL_FRect){
                rect.x + contextmenu->text_indent_x,
                rect.y + contextmenu->text_indent_y,
                0,
                contextmenu->button_h - 2 * contextmenu->text_indent_y
        };

        while ( now ) {
                if ( now->hide )
                        goto next;
                
                SDL_RenderTexture(contextmenu->renderer, now->triggered ? contextmenu->trigger_texture : contextmenu->button_texture, NULL, &rect);

                if ( now->label ) {
                        label_rect.w = label_rect.h * now->label->rect.w / now->label->rect.h;
                        Label_Draw(now->label, NULL, &label_rect);
                } 

                rect.y += contextmenu->button_h;
                label_rect.y += contextmenu->button_h;

                next:
                now = now->next;
        }
}



// ==== Menu buttons ====
void ContextMenu_SetupButtons( CONTEXTMENU *contextmenu, float radius,
                        int32_t width, int32_t height,
                        SDL_Color background, SDL_Color trigger_color,
                        int32_t indent_w, int32_t indent_h,
                        int32_t text_indent_w, int32_t text_indent_h) 
{
        if ( contextmenu == NULL )
                return;

        contextmenu->button_bg_color = background;
        contextmenu->trigger_color = trigger_color;
        
        contextmenu->w = width + 2 * indent_w;
        contextmenu->h = contextmenu->h - 2 * contextmenu->button_indent_y + contextmenu->buttons_count * ( height - contextmenu->button_h )+ 2 * indent_h;

        contextmenu->button_h = height;
        contextmenu->button_w = width;


        contextmenu->button_indent_y = indent_h;
        contextmenu->button_indent_x = indent_w;

        contextmenu->text_indent_y = text_indent_h;
        contextmenu->text_indent_x = text_indent_w;

        contextmenu->button_radius = radius;

        ContextMenu_CreateMenuTexture(contextmenu);
        ContextMenu_CreateButtonTexture(contextmenu);

}



MENU_BUTTON *ContextMenu_SetButton(    CONTEXTMENU *contextmenu, 
                                int id,
                                LABEL *label,
                                bool hide,
                                bool active,
                                void* (*function)(void*, void*) 
                        )
{       
        MENU_BUTTON *button = ContextMenu_GetButton(contextmenu, id);

        if ( button == NULL) {
                button = malloc(sizeof(MENU_BUTTON));
                if ( button == NULL)
                        return NULL;
                        
                if ( contextmenu->buttons ) 
                        contextmenu->buttons->prev = button;
                button->prev = NULL;
                button->next = contextmenu->buttons;
                contextmenu->buttons = button;

                button->id = id;
                contextmenu->buttons_count++;
                contextmenu->h += contextmenu->button_h;
        }
        
        button->triggered = 0;
        button->hide = hide;
        button->active = active;

        button->function = function;
        button->label = label;  

        ContextMenu_CreateMenuTexture(contextmenu);

        return button;
}



MENU_BUTTON *ContextMenu_GetButton(CONTEXTMENU *contextmenu, int id) {
        MENU_BUTTON *now = contextmenu->buttons;
        while ( now ) {
                if ( now->id == id )
                        return now;
                now = now->next;
        }

        return NULL;
}



bool ContextMenu_DelButton(CONTEXTMENU *contextmenu, MENU_BUTTON *button) {
        if ( NULL == contextmenu || NULL == button ) 
                return 0;
        
        if ( button->prev )
                button->prev->next = button->next;
        else 
                contextmenu->buttons = button->next;

        if ( button->next )
                button->next->prev = button->prev;

        contextmenu->buttons_count--;
        contextmenu->h -= contextmenu->button_h;

        free(button);

        return 1;
}



void ContextMenu_HideButton(CONTEXTMENU *contextmenu, MENU_BUTTON *button, bool hide) {
        if ( button == NULL )
                return;

        if ( button->hide == 0 && hide == 1 ) {
                contextmenu->buttons_count--;
                contextmenu->h -= contextmenu->button_h;
        } else if ( button->hide == 1 && hide == 0 ) {
                contextmenu->buttons_count++;
                contextmenu->h += contextmenu->button_h;
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



void __ContextMenu_FillSurfaceBorder(SDL_Surface *surf, Uint32 w, Uint32 h, int32_t border_width, int32_t radius, Uint32 color) {
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



void __ContextMenu_SurfaceFillRect(SDL_Surface *surf, SDL_Rect rect, int32_t radius, Uint32 color) {
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



SDL_Texture *ContextMenu_CreateMenuTexture(CONTEXTMENU *contextmenu) {
        if ( contextmenu == NULL ) 
                return NULL;
      
        SDL_Surface *surf = SDL_CreateSurface(contextmenu->w + 2 * contextmenu->border_width, contextmenu->h + 2 * contextmenu->border_width, contextmenu->pixel_format);
        if ( surf == NULL )
                return NULL;

        Uint32 color = SDL_MapRGBA(SDL_GetPixelFormatDetails(contextmenu->pixel_format), NULL, contextmenu->border_color.r, contextmenu->border_color.g, contextmenu->border_color.b, contextmenu->border_color.a);
        
        if ( contextmenu->border_width )
                __ContextMenu_FillSurfaceBorder(surf, contextmenu->w + 2 * contextmenu->border_width, contextmenu->h + 2 * contextmenu->border_width, contextmenu->border_width, contextmenu->border_radius, color);
        
        SDL_Rect tmp = (SDL_Rect){
                contextmenu->border_width,
                contextmenu->border_width,
                contextmenu->w,
                contextmenu->h
        };

        color = SDL_MapRGBA(SDL_GetPixelFormatDetails(contextmenu->pixel_format), NULL, contextmenu->bg_color.r, contextmenu->bg_color.g, contextmenu->bg_color.b, contextmenu->bg_color.a);

        __ContextMenu_SurfaceFillRect(surf, tmp, contextmenu->border_radius, color);

        contextmenu->menu_texture = SDL_CreateTextureFromSurface(contextmenu->renderer, surf);

        SDL_DestroySurface(surf);

        return contextmenu->menu_texture;
}


bool ContextMenu_CreateButtonTexture(CONTEXTMENU *contextmenu) {
        if ( contextmenu == NULL )
                return NULL;

        SDL_Surface *surf = SDL_CreateSurface(contextmenu->button_w, contextmenu->button_h, contextmenu->pixel_format);
        if ( surf == NULL )
                return NULL;

        SDL_Rect rect = (SDL_Rect){
                0,
                0,
                contextmenu->button_w,
                contextmenu->button_h
        };

        // button texture
        Uint32 color = SDL_MapRGBA( SDL_GetPixelFormatDetails(contextmenu->pixel_format), NULL, 
                                    contextmenu->button_bg_color.r, contextmenu->button_bg_color.g, 
                                    contextmenu->button_bg_color.b, contextmenu->button_bg_color.a);

        __ContextMenu_SurfaceFillRect(surf, rect, contextmenu->button_radius, color);

        contextmenu->button_texture = SDL_CreateTextureFromSurface(contextmenu->renderer, surf);

        // trigger texture
        color = SDL_MapRGBA( SDL_GetPixelFormatDetails(contextmenu->pixel_format), NULL, 
                             contextmenu->trigger_color.r, contextmenu->trigger_color.g, 
                             contextmenu->trigger_color.b, contextmenu->trigger_color.a);

        __ContextMenu_SurfaceFillRect(surf, rect, contextmenu->button_radius, color);

        contextmenu->trigger_texture = SDL_CreateTextureFromSurface(contextmenu->renderer, surf);

        SDL_DestroySurface(surf);

        return contextmenu->button_texture && contextmenu->trigger_texture;
}
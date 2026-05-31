#include "path.h"


// --------------------- render functions ---------------------
void _RenderPointCords(Point *point, LABEL *label, Parametrs *_Parameters) {
        if ( point->state == PSTATE_NONE_STATE ) 
                return;

        char point_text[32];
        snprintf(point_text, sizeof(point_text), "(%.3f, %.3f)", point->cords.x, point->cords.y);
        Label_Update(label, point_text, TEXT_COLOR_Black);

        SDL_FPoint real_cords = BoxCordsToWindow(point->cords, _Parameters);

        SDL_FRect label_rect = {
                real_cords.x + 10,
                real_cords.y + 10,
                label->rect.w / label->rect.h * 30,
                30,
        };

        if ( label_rect.x + label_rect.w + 10 > _Parameters->texture_box.x + _Parameters->texture_box.w ) {
                label_rect.x = real_cords.x - 10 - label_rect.w;
        }

        if ( label_rect.y + label_rect.h + 10 > _Parameters->texture_box.y + _Parameters->texture_box.h ) {
                label_rect.y = real_cords.y - 10 - label_rect.h;
        }

        Label_Draw(label, NULL, &label_rect);
}


void _RenderPoint(SDL_Renderer *renderer, Point *point, SDL_Texture *point_texture, Parametrs *_Parameters) {
        SDL_FPoint real_cords = BoxCordsToWindow(point->cords, _Parameters);
        SDL_FRect pos = {       
                real_cords.x - _Parameters->point_radius, 
                real_cords.y - _Parameters->point_radius, 
                _Parameters->point_diametr, 
                _Parameters->point_diametr
        };

        SDL_RenderTexture(renderer, point_texture, NULL, &pos);

        SDL_FPoint angle_vector = {_Parameters->dir_vector_legth, 0.0};
        angle_vector = Vector_Rotate(angle_vector, point->angle);
        if ( !_Parameters->invert_y )
                angle_vector.y = -angle_vector.y;
        if ( _Parameters->invert_x )
                angle_vector.x = -angle_vector.x;
        angle_vector = Vector_Sum(real_cords, angle_vector);

        if ( point->state == PSTATE_VECTOR_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        } else if ( point->state == PSTATE_VECTOR_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        } else {
                SDL_SetRenderDrawColor(renderer, 160, 0, 160, 255);
        }
        RenderVector(renderer, real_cords, angle_vector, _Parameters->dir_vector_width, _Parameters->dir_vector_arrow_base);
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
}


void _RenderLine(SDL_Renderer *renderer, Point *point, Parametrs *_Parameters) {
        if ( point->state == PSTATE_LINE_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 90, 255, 90, 255);
        } else if ( point->state == PSTATE_LINE_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 255, 90, 90, 255);
        }

        SDL_FPoint window_cords = BoxCordsToWindow(point->cords, _Parameters);
        SDL_FPoint window_next_cords = BoxCordsToWindow(point->next->cords, _Parameters);
        
        RenderLine(renderer, window_cords, window_next_cords, _Parameters->line_width);
        RenderArrow(renderer, window_cords, window_next_cords, _Parameters->line_arrow_base, _Parameters->point_radius);

        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); 
}


void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, Parametrs *_Parameters) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

        Point *now_point = points->points;
        while ( now_point ) {
                if ( now_point->next ) {
                        _RenderLine(renderer, now_point, _Parameters);  
                }
                
                if ( now_point->state == PSTATE_NONE_STATE ) {
                        _RenderPoint(renderer, now_point, point_texture, _Parameters);
                }

                now_point = now_point->next;
        }

        if ( points->selected_point ) {
                switch ( points->selected_point->state ) {
                        case PSTATE_UNDER_MOUSE:
                                SDL_SetTextureColorMod(point_texture, 40, 255, 40);
                                break;
                        case PSTATE_SELECTED:
                                SDL_SetTextureColorMod(point_texture, 255, 40, 40);
                                break;
                        default:
                                SDL_SetTextureColorMod(point_texture, 0, 0, 0);
                                break;
                }

                _RenderPoint(renderer, points->selected_point, point_texture, _Parameters);
                _RenderPointCords(points->selected_point, point_label, _Parameters);

                SDL_SetTextureColorMod(point_texture, 0, 0, 0);
        }
        
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
}
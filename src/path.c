#include "path.h"


bool _PointUnderMouse(SDL_FPoint cords, SDL_FPoint mouse_cords, float r) {
        return Vector_SqAbs(Vector_Sub(cords, mouse_cords)) < r * r;
}




static float start_x = 0;
static float start_y = 0;
static float mouse_point_x = 0;
static float mouse_point_y = 0;

SDL_FPoint CordsToBox(SDL_FPoint cords, SDL_FRect texture_box) {
        cords.x -= texture_box.x;
        cords.y -= texture_box.y;

        cords.x *= BOX_WIDTH / texture_box.w;
        cords.y *= BOX_HEIGHT / texture_box.h;

        return cords;
}

SDL_FPoint CordsToWindow(SDL_FPoint cords, SDL_FRect texture_box) {
        cords.x *= texture_box.w /BOX_WIDTH;
        cords.y *= texture_box.h / BOX_HEIGHT;
        
        cords.x += texture_box.x;
        cords.y += texture_box.y;

        return cords;
}




void _RenderPointCords(Point *point, LABEL *label, SDL_FRect texture_box) {
        if ( point->state == PSTATE_NONE_STATE ) 
                return;

        char point_text[32];
        snprintf(point_text, sizeof(point_text), "(%.3f, %.3f)", point->cords.x, point->cords.y);
        Label_Update(label, point_text, TEXT_COLOR_Black);

        SDL_FPoint real_cords = CordsToWindow(point->cords, texture_box);

        SDL_FRect label_rect = {
                real_cords.x + 10,
                real_cords.y + 10,
                label->rect.w / label->rect.h * 30,
                30,
        };

        if ( label_rect.x + label_rect.w + 10 > texture_box.x + texture_box.w ) {
                label_rect.x = real_cords.x - 10 - label_rect.w;
        }

        if ( label_rect.y + label_rect.h + 10 > texture_box.y + texture_box.h ) {
                label_rect.y = real_cords.y - 10 - label_rect.h;
        }

        Label_Draw(label, NULL, &label_rect);
}


void _RenderPoint(SDL_Renderer *renderer, Point *point, SDL_Texture *point_texture, SDL_FRect texture_box) {
        SDL_FPoint real_cords = CordsToWindow(point->cords, texture_box);
        SDL_FRect pos = {       
                real_cords.x - POINT_RADIUS, 
                real_cords.y - POINT_RADIUS, 
                POINT_DIAMETR, 
                POINT_DIAMETR
        };

        SDL_RenderTexture(renderer, point_texture, NULL, &pos);
}


void _RenderLine(SDL_Renderer *renderer, Point *point, SDL_FColor arrow_Fcolor, SDL_FRect texture_box) {
        if ( point->line_state == PSTATE_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 90, 255, 90, 255);
        } else if ( point->line_state == PSTATE_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 255, 90, 90, 255);
        }

        SDL_FPoint window_cords = CordsToWindow(point->cords, texture_box);
        SDL_FPoint window_next_cords = CordsToWindow(point->next->cords, texture_box);
        
        RenderLine(renderer, window_cords, window_next_cords, LINE_RADIUS);
        RenderArrow(renderer, window_cords, window_next_cords, ARROW_BASE, arrow_Fcolor);

        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); 
}


void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, SDL_FRect texture_box) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

        SDL_Color arrow_color = { 50, 50, 255, 255 };
        SDL_FColor arrow_Fcolor = (SDL_FColor) { arrow_color.r / 255.f, arrow_color.g / 255.f, arrow_color.b / 255.f, arrow_color.a / 255.f };

        Point *now_point = points->points;
        while ( now_point ) {
                if ( now_point->next ) {
                      _RenderLine(renderer, now_point, arrow_Fcolor, texture_box);  
                }
                
                if ( now_point->state == PSTATE_NONE_STATE )
                        _RenderPoint(renderer, now_point, point_texture, texture_box);

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

                _RenderPoint(renderer, points->selected_point, point_texture, texture_box);
                _RenderPointCords(points->selected_point, point_label, texture_box);

                SDL_SetTextureColorMod(point_texture, 0, 0, 0);
        }
        
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
}




void MovePoint(Point *point, SDL_FPoint pos, bool shift_pressed, bool ctrl_pressed, bool alt_pressed) {
        pos.x += mouse_point_x;
        pos.y += mouse_point_y;

        if ( ctrl_pressed && point->prev && point->next ) {
                SDL_FPoint P1M = Vector_Sub(pos, point->prev->cords);
                SDL_FPoint P1P2 = Vector_Sub(point->next->cords, point->prev->cords);

                float k1 = Vector_DotProd(P1M, P1P2) / Vector_SqAbs(P1P2);
                float k2 = 0.5 - k1;

                SDL_FPoint P1 = Vector_Mult_scl(P1P2, k1);
                SDL_FPoint P2 = Vector_Mult_scl(P1P2, k2);

                P1 = Vector_Sum(point->prev->cords, P1);

                if ( Vector_SqAbs( Vector_Sub(pos, P1) ) < Vector_SqAbs(P2) )
                        pos = P1;
                else
                        pos = Vector_Sum(pos, P2);
        } else if ( alt_pressed && shift_pressed && point->next ) {
                SDL_FPoint next_cords = point->next->cords;
                SDL_FPoint Vd = Vector_Sub(pos, next_cords);
                if ( fabs(Vd.x) < fabs(Vd.y) )
                        pos.x = next_cords.x;
                else
                        pos.y = next_cords.y;
        } else if ( alt_pressed && point->prev ) {
                SDL_FPoint prev_cords = point->prev->cords;
                SDL_FPoint Vd = Vector_Sub(pos, prev_cords);
                if ( fabs(Vd.x) < fabs(Vd.y) )
                        pos.x = prev_cords.x;
                else
                        pos.y = prev_cords.y;
        } else if ( shift_pressed ) {
                if ( fabs(pos.x-start_x) > fabs(pos.y-start_y) ) {
                        pos.y = start_y;
                } else {
                        pos.x = start_x;
                }
        }

        if ( pos.x < 0 )
                pos.x = 0;
        else if ( pos.x > BOX_WIDTH )
                pos.x = BOX_WIDTH;


        if ( pos.y < 0 )
                pos.y = 0;
        else if ( pos.y > BOX_HEIGHT )
                pos.y = BOX_HEIGHT;


        point->cords = pos;
}


bool TouchLine(SDL_FPoint P1, SDL_FPoint P2, SDL_FPoint M, float line_r, float point_r) {
        SDL_FPoint A = Vector_Sub(M, P1);
        SDL_FPoint B = Vector_Sub(M, P2);
        SDL_FPoint C = Vector_Sub(P2, P1);

        float SqAbs_A = Vector_SqAbs(A);
        float SqAbs_B = Vector_SqAbs(B);
        float SqAbs_C = Vector_SqAbs(C);
        float sq_point_r = point_r * point_r;

        if ( SqAbs_A >= SqAbs_C || SqAbs_B >= SqAbs_C || SqAbs_A <= sq_point_r || SqAbs_B <= sq_point_r )
                return 0;

        double tmp = A.x * C.y - A.y * C.x;

        return tmp * tmp / SqAbs_C <= line_r * line_r;

}


bool CheckLine(PArray *points, Point *point, SDL_FPoint mouse_pos, bool mouse_pressed, bool prev_mouse_state, float fixed_radius, float fixed_line_radius) {
        PState new = PSTATE_NONE_STATE;

        if (    
                TouchLine(point->cords, point->next->cords, mouse_pos, fixed_line_radius, fixed_radius)
        ) {
                if ( mouse_pressed && prev_mouse_state == 0 ) {
                        new = PSTATE_SELECTED;
                } else {
                        new = PSTATE_UNDER_MOUSE;
                }
                points->selected_line = point;
        }

        if ( point->line_state != new ) {
                point->line_state = new;
                
                return 1;
        }
        
        return 0;
}


bool CheckPoint(PArray *points, Point *point, SDL_FPoint mouse_pos, bool mouse_pressed, bool prev_mouse_state, float fixed_radius) {
        PState new = PSTATE_NONE_STATE;

        if ( _PointUnderMouse(point->cords, mouse_pos, fixed_radius) ) {
                if ( mouse_pressed && prev_mouse_state == 0 ) {
                        new = PSTATE_SELECTED;
                        start_x = point->cords.x;
                        start_y = point->cords.y;

                        mouse_point_x = start_x - mouse_pos.x;
                        mouse_point_y = start_y - mouse_pos.y;
                } else {
                        new = PSTATE_UNDER_MOUSE;
                }
                points->selected_point = point;
        }


        if ( point->state != new ) {
                point->state = new;
                return 1;
        }

        return 0;
}


bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, SDL_FRect texture_box, bool mouse_pressed, bool prev_mouse_state, bool shift_pressed, bool ctrl_pressed, bool alt_pressed) {
        float fixed_radius = POINT_RADIUS * BOX_HEIGHT / texture_box.h;
        float fixed_line_radius = LINE_RADIUS * BOX_HEIGHT / texture_box.h;

        mouse_pos = CordsToBox(mouse_pos, texture_box);

        if ( points->selected_point ) {
                points->selected_line = NULL;
                if ( points->selected_point->state == PSTATE_SELECTED && mouse_pressed ) {
                        MovePoint(points->selected_point, mouse_pos, shift_pressed, ctrl_pressed, alt_pressed);
                } else {
                        points->selected_point->state = PSTATE_NONE_STATE;
                        points->selected_point = NULL;
                } 
                points->changed = 1;
        }

        if ( points->selected_line ) {
                if ( mouse_pressed == 0 || points->selected_line->line_state != PSTATE_SELECTED ) {
                        points->selected_line->line_state = PSTATE_NONE_STATE;
                        points->selected_line = NULL;
                        points->changed = 1;
                }
        }
        
        Point *now_point = points->points;
        while ( now_point ) {
                if ( points->selected_point || points->selected_line ) {
                        break;
                }

                points->changed |= CheckPoint(points, now_point, mouse_pos, mouse_pressed, prev_mouse_state, fixed_radius);

                if ( points->selected_point == NULL && now_point->next ) {
                        points->changed |= CheckLine(points, now_point, mouse_pos, mouse_pressed, prev_mouse_state, fixed_radius, fixed_line_radius);
                }
                now_point = now_point->next;
        }
        
        SDL_SetCursor(SDL_CreateSystemCursor(points->selected_point ? points->selected_point->state == PSTATE_UNDER_MOUSE ? SDL_SYSTEM_CURSOR_POINTER : SDL_SYSTEM_CURSOR_MOVE : SDL_SYSTEM_CURSOR_DEFAULT ));

        return points->changed;
} 




void AddPoint(PArray *points, SDL_FPoint cords, Point *line) {
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
        }

        if ( cords.x < 0 )
                cords.x = 0;
        else if ( cords.x > BOX_WIDTH )
                cords.x = BOX_WIDTH;


        if ( cords.y < 0 )
                cords.y = 0;
        else if ( cords.y > BOX_HEIGHT )
                cords.y = BOX_HEIGHT;

        *new = (Point){
                cords, 
                PSTATE_NONE_STATE, PSTATE_NONE_STATE,
                NULL,
                NULL
        };

        if ( line && line->next ) {
                SDL_FPoint ac = Vector_Sub( cords, line->cords );
                SDL_FPoint ab = Vector_Sub( line->next->cords, line->cords );

                double k = Vector_DotProd(ac, ab);
                k /= Vector_SqAbs(ab);

                SDL_FPoint result = Vector_Mult_scl(ab, k);
                result = Vector_Sum(line->cords, result);

                new->cords = result;

                new->next = line->next;
                new->prev = line;
                line->next->prev = new;
                line->next = new;
                
                return;
        }

        Point *now = points->points;
        if ( now == NULL ) {
                points->points = new;
                return;
        }

        while ( now->next ) {
                now = now->next;
        }

        now->next = new;
        new->prev = now;
}

void AddPoint_tostart(PArray *points, SDL_FPoint cords) {
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
        }

        if ( cords.x < 0 )
                cords.x = 0;
        else if ( cords.x > BOX_WIDTH )
                cords.x = BOX_WIDTH;


        if ( cords.y < 0 )
                cords.y = 0;
        else if ( cords.y > BOX_HEIGHT )
                cords.y = BOX_HEIGHT;
                
        *new = (Point){
                cords, 
                PSTATE_NONE_STATE, PSTATE_NONE_STATE,
                NULL,
                NULL
        }; 

        new->next = points->points;
        points->points = new;
        if ( new->next ) {
                new->next->prev = new;
        }
}

void DelPoint(PArray *points, Point *point) {
        if ( point->prev ) {
                point->prev->next = point->next;
        } else {
                points->points = point->next;
        }

        if ( point->next ) {
                point->next->prev = point->prev;
        }

        free(point);
}



void FreePoints(PArray *_Points) {
        Point *now = _Points->points;

        while ( now ) {
                Point *next = now->next;
                free(now);
                now = next;
        }
}
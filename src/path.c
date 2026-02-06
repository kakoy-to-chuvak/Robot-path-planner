#include "path.h"

#define SGN(X) ( (X) > 0 ? 1 : -1 )
#define COS_PI_8 (0.92387953251128675612818318939679f)   // cos(PI/8)
#define COS_3_PI_8 (0.3826834323650897717284599840304f)  // cos(PI*3/8)

// --------------------- helper functions ---------------------
bool _PointUnderMouse(SDL_FPoint cords, SDL_FPoint mouse_cords, float r) {
        return Vector_SqAbs(Vector_Sub(cords, mouse_cords)) < r * r;
}

double Safe_Angle(SDL_FPoint vec) {
        if ( vec.x || vec.y ) {
                vec.y = -vec.y;
                float cos_a = Vector_Cos(vec, (SDL_FPoint){0, 1});
                if ( cos_a < -1 ) {
                        return M_PI;
                } else if ( cos_a > 1 ) {
                        return 0; 
                } else {
                        return acos( cos_a ) * SGN(vec.x);
                }
        } else {
                return 0;
        }
}

SDL_FPoint CordsToBox(SDL_FPoint cords, Parametrs *_Parametrs) {
        cords.x -= _Parametrs->texture_box.x;
        cords.y -= _Parametrs->texture_box.y;

        cords.x *= _Parametrs->box_width / _Parametrs->texture_box.w;
        cords.y *= _Parametrs->box_height / _Parametrs->texture_box.h;

        return cords;
}

SDL_FPoint CordsToWindow(SDL_FPoint cords, Parametrs *_Parametrs) {
        cords.x *= _Parametrs->texture_box.w / _Parametrs->box_width;
        cords.y *= _Parametrs->texture_box.h / _Parametrs->box_height;
        
        cords.x += _Parametrs->texture_box.x;
        cords.y += _Parametrs->texture_box.y;

        return cords;
}




// --------------------- render functions ---------------------
void _RenderPointCords(Point *point, LABEL *label, Parametrs *_Parametrs) {
        if ( point->state == PSTATE_NONE_STATE ) 
                return;

        char point_text[32];
        snprintf(point_text, sizeof(point_text), "(%.3f, %.3f)", point->cords.x, point->cords.y);
        Label_Update(label, point_text, TEXT_COLOR_Black);

        SDL_FPoint real_cords = CordsToWindow(point->cords, _Parametrs);

        SDL_FRect label_rect = {
                real_cords.x + 10,
                real_cords.y + 10,
                label->rect.w / label->rect.h * 30,
                30,
        };

        if ( label_rect.x + label_rect.w + 10 > _Parametrs->texture_box.x + _Parametrs->texture_box.w ) {
                label_rect.x = real_cords.x - 10 - label_rect.w;
        }

        if ( label_rect.y + label_rect.h + 10 > _Parametrs->texture_box.y + _Parametrs->texture_box.h ) {
                label_rect.y = real_cords.y - 10 - label_rect.h;
        }

        Label_Draw(label, NULL, &label_rect);
}


void _RenderPoint(SDL_Renderer *renderer, Point *point, SDL_Texture *point_texture, Parametrs *_Parametrs) {
        SDL_FPoint real_cords = CordsToWindow(point->cords, _Parametrs);
        SDL_FRect pos = {       
                real_cords.x - _Parametrs->point_radius, 
                real_cords.y - _Parametrs->point_radius, 
                _Parametrs->point_diametr, 
                _Parametrs->point_diametr
        };

        SDL_RenderTexture(renderer, point_texture, NULL, &pos);

        SDL_FPoint angle_vector = {0, _Parametrs->dir_vector_legth};
        angle_vector = Vector_Rotate(angle_vector, point->angle);
        angle_vector.y = -angle_vector.y;
        angle_vector = Vector_Sum(real_cords, angle_vector);

        if ( point->state == PSTATE_VECTOR_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        } else if ( point->state == PSTATE_VECTOR_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        } else {
                SDL_SetRenderDrawColor(renderer, 160, 0, 160, 255);
        }
        RenderVector(renderer, real_cords, angle_vector, _Parametrs->dir_vector_width, _Parametrs->dir_vector_arrow_base);
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
}


void _RenderLine(SDL_Renderer *renderer, Point *point, Parametrs *_Parametrs) {
        if ( point->state == PSTATE_LINE_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 90, 255, 90, 255);
        } else if ( point->state == PSTATE_LINE_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 255, 90, 90, 255);
        }

        SDL_FPoint window_cords = CordsToWindow(point->cords, _Parametrs);
        SDL_FPoint window_next_cords = CordsToWindow(point->next->cords, _Parametrs);
        
        RenderLine(renderer, window_cords, window_next_cords, _Parametrs->line_width);
        RenderArrow(renderer, window_cords, window_next_cords, _Parametrs->line_arrow_base, _Parametrs->point_radius);

        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); 
}


void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, Parametrs *_Parametrs) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

        Point *now_point = points->points;
        while ( now_point ) {
                if ( now_point->next ) {
                        _RenderLine(renderer, now_point, _Parametrs);  
                }
                
                if ( now_point->state == PSTATE_NONE_STATE ) {
                        _RenderPoint(renderer, now_point, point_texture, _Parametrs);
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

                _RenderPoint(renderer, points->selected_point, point_texture, _Parametrs);
                _RenderPointCords(points->selected_point, point_label, _Parametrs);

                SDL_SetTextureColorMod(point_texture, 0, 0, 0);
        }
        
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
}





// --------------------- main functions ---------------------
static SDL_FPoint start_point;
static SDL_FPoint mouse_point;

SDL_FPoint _GetStraightPos( SDL_FPoint pos, SDL_FPoint source_point ) {         // fixing position in 8 directions (S,W,E,N,SW,SE,NW,NE):
        SDL_FPoint Vd = Vector_Sub(pos, source_point);                          //     *         ^ y       *
        float cos_alfa = Vector_Cos(Vd, (SDL_FPoint){0, 1});                    //       *       |       *
                                                                                //         *     |     *
        if ( fabs(cos_alfa) > COS_PI_8 ) {                                      //           *   |   *
                pos.x = source_point.x;                                         //             * | *
        } else if ( fabs(cos_alfa) < COS_3_PI_8 ) {                             //---------------*-------------->
                pos.y = source_point.y;                                         //             * | *            x
        } else if ( SGN( Vd.y ) == SGN( Vd.x ) ) {                              //           *   |   *
                Vd.x = ( Vd.x + Vd.y ) / 2;                                     //         *     |     *
                Vd.y = Vd.x;                                                    //       *       |       *
                pos = Vector_Sum(Vd, source_point);                             //     *         |         *
        } else {
                Vd.x = ( Vd.x - Vd.y ) / 2;
                Vd.y = -Vd.x;
                pos = Vector_Sum(Vd, source_point);
        }

        return pos;
}

void MovePoint( Point *point, SDL_FPoint pos, Parametrs *_Parametrs ) {
        

        if ( _Parametrs->ctrl_pressed && point->prev && point->next ) {
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
        } else if ( _Parametrs->alt_pressed && _Parametrs->shift_pressed && point->next ) {
                pos = _GetStraightPos(pos, point->next->cords);
        } else if ( _Parametrs->alt_pressed && point->prev ) {
                pos = _GetStraightPos(pos, point->prev->cords);
        } else if ( _Parametrs->shift_pressed ) {
                pos = _GetStraightPos(pos, start_point);
        } else {
                pos = Vector_Sum(pos, mouse_point);
        }

        if ( pos.x < 0 )
                pos.x = 0;
        else if ( pos.x > _Parametrs->box_width )
                pos.x = _Parametrs->box_width;


        if ( pos.y < 0 )
                pos.y = 0;
        else if ( pos.y > _Parametrs->box_height )
                pos.y = _Parametrs->box_height;


        point->cords = pos;
}


bool _TouchLine(SDL_FPoint P1, SDL_FPoint P2, SDL_FPoint M, float line_r, float point_r ) {
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


bool CheckLine(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parametrs ) {
        PState new = PSTATE_NONE_STATE;

        if (    
                _TouchLine(point->cords, point->next->cords, mouse_pos, _Parametrs->fixed_line_width, _Parametrs->fixed_point_radius)
        ) {
                if ( _Parametrs->lmb_pressed && _Parametrs->prev_lmb_state == 0 ) {
                        new = PSTATE_LINE_SELECTED;
                } else {
                        new = PSTATE_LINE_UNDER_MOUSE;
                }
                points->selected_point = point;
        }

        if ( point->state != new ) {
                point->state = new;
                
                return 1;
        }
        
        return 0;
}


bool CheckPoint(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parametrs ) {
        PState new = PSTATE_NONE_STATE;

        if ( _PointUnderMouse(point->cords, mouse_pos, _Parametrs->fixed_point_radius) ) {
                if ( _Parametrs->lmb_pressed && _Parametrs->prev_lmb_state == 0 ) {
                        new = PSTATE_SELECTED;
                        start_point = point->cords;

                        mouse_point = Vector_Sub(start_point, mouse_pos);
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

bool _TouchVector(SDL_FPoint _Start, SDL_FPoint _Mouse_pos, float angle, float width, float len ) {
        SDL_FPoint _End = Vector_Rotate( (SDL_FPoint){0, len}, angle);
        _End.y = -_End.y;
        _End = Vector_Sum(_Start, _End);
        return _TouchLine(_Start, _End, _Mouse_pos, width, 0);
}


bool CheckVector(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parametrs ) {
        PState new = PSTATE_NONE_STATE;

        if (    
                _TouchVector(point->cords, mouse_pos, point->angle, _Parametrs->fixed_dir_vector_width, _Parametrs->fixed_dir_vector_legth)
        ) {
                if ( _Parametrs->lmb_pressed && _Parametrs->prev_lmb_state == 0 ) {
                        new = PSTATE_VECTOR_SELECTED;
                } else {
                        new = PSTATE_VECTOR_UNDER_MOUSE;
                }
                points->selected_point = point;
        }

        if ( point->state != new ) {
                point->state = new;
                
                return 1;
        }
        
        return 0;
}

void MoveVector(Point *point, SDL_FPoint pos) {
        SDL_FPoint Dv = Vector_Sub(pos, point->cords);
        point->angle = Safe_Angle(Dv);
}


void CheckSelectedPoint(  PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parametrs ) {
        switch ( points->selected_point->state ) {
                case PSTATE_SELECTED:
                        if ( _Parametrs->lmb_pressed ) {
                                MovePoint(points->selected_point, mouse_pos, _Parametrs);
                        } else if ( _PointUnderMouse(points->selected_point->cords, mouse_pos, _Parametrs->fixed_point_radius) ) {
                                points->selected_point->state = PSTATE_UNDER_MOUSE;
                        } else {
                                points->selected_point->state = PSTATE_NONE_STATE;
                                points->selected_point = NULL;
                        }
                        break;
                case PSTATE_LINE_SELECTED:
                        if ( _Parametrs->lmb_pressed == 0 ) {
                                if (    points->selected_point->next && 
                                        _TouchLine(points->selected_point->cords, points->selected_point->next->cords, mouse_pos, _Parametrs->fixed_line_width, _Parametrs->fixed_point_radius) ) 
                                {
                                        points->selected_point->state = PSTATE_LINE_UNDER_MOUSE;
                                } else {
                                        points->selected_point->state = PSTATE_NONE_STATE;
                                        points->selected_point = NULL;
                                }
                        }
                        break;
                case PSTATE_VECTOR_SELECTED:
                        if ( _Parametrs->lmb_pressed ) {
                                MoveVector(points->selected_point, mouse_pos);
                        } else if ( 
                                _TouchVector(points->selected_point->cords, mouse_pos, points->selected_point->angle, _Parametrs->fixed_dir_vector_width, _Parametrs->fixed_dir_vector_legth)
                        ) {
                                points->selected_point->state = PSTATE_VECTOR_UNDER_MOUSE;
                        } else {
                                points->selected_point->state = PSTATE_NONE_STATE;
                                points->selected_point = NULL;
                        }
                        break;
                default:
                        points->selected_point->state = PSTATE_NONE_STATE;
                        points->selected_point = NULL;
                        break;
        }
}



bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parametrs) {
        mouse_pos = CordsToBox(mouse_pos, _Parametrs);

        if ( points->selected_point ) {
                CheckSelectedPoint( points, mouse_pos, _Parametrs );
                points->changed = 1;
        }
        
        Point *now_point = points->points;
        while ( now_point ) {
                if ( points->selected_point ) {
                        break;
                }

                points->changed |= CheckVector(points, now_point, mouse_pos, _Parametrs );

                if ( points->selected_point == NULL )
                        points->changed |= CheckPoint(points, now_point, mouse_pos, _Parametrs );

                if ( points->selected_point == NULL && now_point->next ) {
                        points->changed |= CheckLine(points, now_point, mouse_pos, _Parametrs );
                }
                now_point = now_point->next;
        }
        
        SDL_Cursor *cursor = NULL;
        
        if ( points->selected_point ) {
                if (    points->selected_point->state == PSTATE_LINE_UNDER_MOUSE ||
                        points->selected_point->state == PSTATE_UNDER_MOUSE ||
                        points->selected_point->state == PSTATE_VECTOR_UNDER_MOUSE || 
                        points->selected_point->state == PSTATE_LINE_SELECTED ) 
                {
                        cursor = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_POINTER );
                } else {
                        cursor = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_MOVE );
                }
        } else {
                cursor = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_DEFAULT );
        }
        SDL_SetCursor(cursor);

        return points->changed;
} 







void AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line, Parametrs *_Parametrs) {
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
                return;
        }

        if ( cords.x < 0 )
                cords.x = 0;
        else if ( cords.x > _Parametrs->box_width )
                cords.x = _Parametrs->box_width;


        if ( cords.y < 0 )
                cords.y = 0;
        else if ( cords.y > _Parametrs->box_height )
                cords.y = _Parametrs->box_height;

        *new = (Point){
                cords,
                0,
                PSTATE_NONE_STATE,
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

                SDL_FPoint Dv = Vector_Sub(new->cords, new->prev->cords);
                new->angle = Safe_Angle(Dv);
                
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

        if ( angle ) {
                new->angle = *angle;
        } else if ( new->prev ) {
                SDL_FPoint Dv = Vector_Sub(new->cords, new->prev->cords);
                new->angle = Safe_Angle(Dv);
        }
}

void AddPoint_tostart(PArray *points, SDL_FPoint cords, float angle, Parametrs * _Parametrs) {
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
        }

        if ( cords.x < 0 )
                cords.x = 0;
        else if ( cords.x > _Parametrs->box_width )
                cords.x = _Parametrs->box_width;


        if ( cords.y < 0 )
                cords.y = 0;
        else if ( cords.y > _Parametrs->box_height )
                cords.y = _Parametrs->box_height;
                
        *new = (Point){
                cords,
                0,
                PSTATE_NONE_STATE,
                NULL,
                NULL
        }; 

        new->next = points->points;
        points->points = new;
        if ( new->next ) {
                new->next->prev = new;
        }

        new->angle = angle;
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
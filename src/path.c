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
                float cos_a = Vector_Cos(vec, (SDL_FPoint){1, 0});
                if ( cos_a < -1 ) {
                        return M_PI;
                } else if ( cos_a > 1 ) {
                        return 0; 
                }
                return acos( cos_a ) * SGN(vec.y);
        }
        return 0;
}

SDL_FPoint WindowCordsToBox(SDL_FPoint cords, Parametrs *_Parameters) {
        cords.x -= _Parameters->texture_box.x;
        cords.y -= _Parameters->texture_box.y;

        cords.x *= _Parameters->field_width / _Parameters->texture_box.w;
        cords.y *= _Parameters->field_height / _Parameters->texture_box.h;

        if ( _Parameters->invert_x ) {
                cords.x = _Parameters->field_width - cords.x;
        }
        if ( _Parameters->invert_y ) {
                cords.y = _Parameters->field_height - cords.y;
        }

        cords.x -= _Parameters->field_cord_center.x;
        cords.y -= _Parameters->field_cord_center.y;

        return cords;
}

SDL_FPoint BoxCordsToWindow(SDL_FPoint cords, Parametrs *_Parameters) {
        cords.x += _Parameters->field_cord_center.x;
        cords.y += _Parameters->field_cord_center.y;

        if ( _Parameters->invert_x ) {
                cords.x = _Parameters->field_width - cords.x;
        }
        if ( _Parameters->invert_y ) {
                cords.y = _Parameters->field_height - cords.y;
        }

        cords.x *= _Parameters->texture_box.w / _Parameters->field_width;
        cords.y *= _Parameters->texture_box.h / _Parameters->field_height;
        
        cords.x += _Parameters->texture_box.x;
        cords.y += _Parameters->texture_box.y;

        return cords;
}




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

void MovePoint( Point *point, SDL_FPoint pos, Parametrs *_Parameters ) {
        if ( _Parameters->alt_pressed && _Parameters->ctrl_pressed && point->next ) {
                pos = _GetStraightPos(pos, point->next->cords);
        } else if ( _Parameters->alt_pressed && point->prev ) {
                pos = _GetStraightPos(pos, point->prev->cords);
        } else if ( _Parameters->ctrl_pressed && point->prev && point->next ) {
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
        } else if ( _Parameters->shift_pressed ) {
                pos = _GetStraightPos(pos, start_point);
        } else {
                pos = Vector_Sum(pos, mouse_point);
        }

        // fix cords
        if ( pos.x < -_Parameters->field_cord_center.x )
                pos.x = -_Parameters->field_cord_center.x;
        else if ( pos.x > _Parameters->field_width - _Parameters->field_cord_center.x )
                pos.x = _Parameters->field_width - _Parameters->field_cord_center.x;


        if ( pos.y < -_Parameters->field_cord_center.y )
                pos.y = -_Parameters->field_cord_center.y;
        else if ( pos.y > _Parameters->field_height - _Parameters->field_cord_center.y )
                pos.y = _Parameters->field_height - _Parameters->field_cord_center.y;


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


bool CheckLine(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parameters ) {
        PState new = PSTATE_NONE_STATE;

        if (    
                _TouchLine(point->cords, point->next->cords, mouse_pos, _Parameters->fixed_line_width, _Parameters->fixed_point_radius)
        ) {
                if ( _Parameters->lmb_pressed && _Parameters->prev_lmb_state == 0 ) {
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


bool CheckPoint(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parameters ) {
        PState new = PSTATE_NONE_STATE;

        if ( _PointUnderMouse(point->cords, mouse_pos, _Parameters->fixed_point_radius) ) {
                if ( _Parameters->lmb_pressed && _Parameters->prev_lmb_state == 0 ) {
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

bool _TouchVector(SDL_FPoint _Start, SDL_FPoint _Mouse_pos, float angle, Parametrs *_Parameters ) {
        SDL_FPoint _End = Vector_Rotate( (SDL_FPoint){_Parameters->fixed_dir_vector_legth, 0}, angle);
        _End.y = -_End.y;
        _End = Vector_Sum(_Start, _End);
        return _TouchLine(_Start, _End, _Mouse_pos, _Parameters->fixed_dir_vector_width, 0);
}


bool CheckVector(PArray *points, Point *point, SDL_FPoint mouse_pos, Parametrs *_Parameters ) {
        PState new = PSTATE_NONE_STATE;

        if (    
                _TouchVector(point->cords, mouse_pos, point->angle, _Parameters)
        ) {
                if ( _Parameters->lmb_pressed && _Parameters->prev_lmb_state == 0 ) {
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

void MoveVector(Point *point, SDL_FPoint mouse_pos, Parametrs *_Parameters ) {
        SDL_FPoint Dv = Vector_Sub(mouse_pos, point->cords);
        point->angle = Safe_Angle(Dv);
        // printf("{%f;%f} %f\n", Dv.x, Dv.y, point->angle);

        if ( _Parameters->alt_pressed && _Parameters->ctrl_pressed && point->next ) {
                Dv = Vector_Sub(point->next->cords, point->cords);
                float angle2 = Safe_Angle(Dv);
                point->angle = round( (point->angle - angle2 ) / M_PI_4 ) * M_PI_4 + angle2;
        } else if ( _Parameters->shift_pressed ) {
                point->angle = round(point->angle / M_PI_4 ) * M_PI_4;
        } else if ( _Parameters->alt_pressed && point->prev ) {
                Dv = Vector_Sub(point->cords, point->prev->cords);
                float angle2 = Safe_Angle(Dv);
                point->angle = round( (point->angle - angle2 ) / M_PI_4 ) * M_PI_4 + angle2;
        } else if ( _Parameters->ctrl_pressed && point->prev && point->next ) {
                Dv = Vector_Sub(point->prev->cords, point->next->cords);
                float angle2 = Safe_Angle(Dv);
                point->angle = round( (point->angle - angle2 ) / M_PI_4 ) * M_PI_4 + angle2;
        }
}


void CheckSelectedPoint(  PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parameters ) {
        switch ( points->selected_point->state ) {
                case PSTATE_SELECTED:
                        if ( _Parameters->lmb_pressed ) {
                                MovePoint(points->selected_point, mouse_pos, _Parameters);
                        } else if ( _PointUnderMouse(points->selected_point->cords, mouse_pos, _Parameters->fixed_point_radius) ) {
                                points->selected_point->state = PSTATE_UNDER_MOUSE;
                        } else {
                                points->selected_point->state = PSTATE_NONE_STATE;
                                points->selected_point = NULL;
                        }
                        break;
                case PSTATE_LINE_SELECTED:
                        if ( _Parameters->lmb_pressed == 0 ) {
                                if (    points->selected_point->next && 
                                        _TouchLine(points->selected_point->cords, points->selected_point->next->cords, mouse_pos, _Parameters->fixed_line_width, _Parameters->fixed_point_radius) ) 
                                {
                                        points->selected_point->state = PSTATE_LINE_UNDER_MOUSE;
                                } else {
                                        points->selected_point->state = PSTATE_NONE_STATE;
                                        points->selected_point = NULL;
                                }
                        }
                        break;
                case PSTATE_VECTOR_SELECTED:
                        if ( _Parameters->lmb_pressed ) {
                                MoveVector(points->selected_point, mouse_pos, _Parameters);
                        } else if ( 
                                _TouchVector(points->selected_point->cords, mouse_pos, points->selected_point->angle, _Parameters)
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



bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, Parametrs *_Parameters) {
        mouse_pos = WindowCordsToBox(mouse_pos, _Parameters);

        if ( points->selected_point ) {
                CheckSelectedPoint( points, mouse_pos, _Parameters );
                points->changed = 1;
        }
        
        Point *now_point = points->points;
        while ( now_point ) {
                if ( points->selected_point ) {
                        break;
                }

                points->changed |= CheckVector(points, now_point, mouse_pos, _Parameters );

                if ( points->selected_point == NULL )
                        points->changed |= CheckPoint(points, now_point, mouse_pos, _Parameters );

                if ( points->selected_point == NULL && now_point->next ) {
                        points->changed |= CheckLine(points, now_point, mouse_pos, _Parameters );
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







Point * AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line, UserField *_Fields, Parametrs *_Parameters) {
        if ( points == NULL || _Parameters == NULL ) {
                return NULL;
        }
        
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
                return NULL;
        }

        // fix cords
        if ( cords.x < -_Parameters->field_cord_center.x )
                cords.x = -_Parameters->field_cord_center.x;
        else if ( cords.x > _Parameters->field_width - _Parameters->field_cord_center.x )
                cords.x = _Parameters->field_width - _Parameters->field_cord_center.x;


        if ( cords.y < -_Parameters->field_cord_center.y )
                cords.y = -_Parameters->field_cord_center.y;
        else if ( cords.y > _Parameters->field_height - _Parameters->field_cord_center.y )
                cords.y = _Parameters->field_height - _Parameters->field_cord_center.y;

        *new = (Point){
                cords,
                0,
                PSTATE_NONE_STATE,
                _Fields,
                NULL,
                NULL
        };

        
        // vector magic to spawn point on line
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
                
                return NULL;
        }

        // add point to end of the PArray
        Point *now = points->points;
        if ( now == NULL ) {
                points->points = new;
                if ( angle ) {
                        new->angle = *angle;
                }
                points->count++;
                return NULL;
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
        points->count++;

        return new;
}

Point *AddPoint_tostart(PArray *points, SDL_FPoint cords, float angle, UserField *_Fields, Parametrs * _Parameters) {
        if ( points == NULL || _Parameters == NULL ) {
                return NULL;
        }

        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
                return NULL;
        }

        // fix cords
        if ( cords.x < -_Parameters->field_cord_center.x )
                cords.x = -_Parameters->field_cord_center.x;
        else if ( cords.x > _Parameters->field_width - _Parameters->field_cord_center.x )
                cords.x = _Parameters->field_width - _Parameters->field_cord_center.x;


        if ( cords.y < -_Parameters->field_cord_center.y )
                cords.y = -_Parameters->field_cord_center.y;
        else if ( cords.y > _Parameters->field_height - _Parameters->field_cord_center.y )
                cords.y = _Parameters->field_height - _Parameters->field_cord_center.y;
                
        *new = (Point){
                cords,
                0,
                PSTATE_NONE_STATE,
                _Fields,
                NULL,
                NULL
        }; 

        new->next = points->points;
        points->points = new;
        if ( new->next ) {
                new->next->prev = new;
        }

        new->angle = angle;
        points->count++;

        return new;
}

void FreeUserFields(UserField *_Field) {
        while ( _Field ) {
                if ( _Field->key )
                        free(_Field->key);
                if ( _Field->value )
                        free(_Field->value);
                UserField *tmp = _Field->next;
                free(_Field);
                _Field = tmp;
        }
}

void FreePoint(Point *_Point) {
        FreeUserFields(_Point->user_fields);
        free(_Point);
}

void DelPoint(PArray *points, Point *point) {
        if ( points == NULL || point == NULL ) {
                return;
        }

        if ( point->prev ) {
                point->prev->next = point->next;
        } else {
                points->points = point->next;
        }

        if ( point->next ) {
                point->next->prev = point->prev;
        }

        FreePoint(point);
        points->count--;
}



void FreePoints(PArray *_Points) {
        if ( _Points == NULL ) {
                return;
        }

        Point *now = _Points->points;

        while ( now ) {
                Point *next = now->next;
                FreePoint(now);
                now = next;
        }

        _Points->points = NULL;
        _Points->changed = 1;
        _Points->selected_point = NULL;
        _Points->count = 0;
}
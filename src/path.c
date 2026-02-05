#include "path.h"

#define SGN(X) ( (X) > 0 ? 1 : -1 )
#define COS_PI_8 (0.92387953251128675612818318939679f)   // cos(PI/8)
#define COS_3_PI_8 (0.3826834323650897717284599840304f)  // cos(PI*3/8)


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



static SDL_FPoint start_point;
static SDL_FPoint mouse_point;

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

        SDL_FPoint angle_vector = {0, POINT_DIAMETR};
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
        RenderVector(renderer, real_cords, angle_vector, DIR_VECTOR_WIDTH, DIR_VECTOR_ARROW_SIZE);
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
}


void _RenderLine(SDL_Renderer *renderer, Point *point, SDL_FRect texture_box) {
        if ( point->state == PSTATE_LINE_UNDER_MOUSE ) {
                SDL_SetRenderDrawColor(renderer, 90, 255, 90, 255);
        } else if ( point->state == PSTATE_LINE_SELECTED ) {
                SDL_SetRenderDrawColor(renderer, 255, 90, 90, 255);
        }

        SDL_FPoint window_cords = CordsToWindow(point->cords, texture_box);
        SDL_FPoint window_next_cords = CordsToWindow(point->next->cords, texture_box);
        
        RenderLine(renderer, window_cords, window_next_cords, LINE_RADIUS);
        RenderArrow(renderer, window_cords, window_next_cords, ARROW_BASE, POINT_RADIUS);

        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); 
}


void RenderPath(SDL_Renderer *renderer, SDL_Texture *point_texture, PArray *points, LABEL *point_label, SDL_FRect texture_box) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

        Point *now_point = points->points;
        while ( now_point ) {
                if ( now_point->next ) {
                        _RenderLine(renderer, now_point, texture_box);  
                }
                
                if ( now_point->state == PSTATE_NONE_STATE ) {
                        _RenderPoint(renderer, now_point, point_texture, texture_box);
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

                _RenderPoint(renderer, points->selected_point, point_texture, texture_box);
                _RenderPointCords(points->selected_point, point_label, texture_box);

                SDL_SetTextureColorMod(point_texture, 0, 0, 0);
        }
        
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
}






SDL_FPoint _GetStraightPos(SDL_FPoint pos, SDL_FPoint source_point) {           // fixing position in 8 directions (S,W,E,N,SW,SE,NW,NE):
        SDL_FPoint Vd = Vector_Sub(pos, source_point);                          //     *         ^ y       *
        float cos_alfa = Vector_Cos(Vd, (SDL_FPoint){0, 1});                    //       *       |       *
                                                                                //         *     |     *
        if ( fabs(cos_alfa) > COS_PI_8 ) {                                      //           *   |   *
                pos.x = source_point.x;                                         //             * | *
        } else if ( fabs(cos_alfa) < COS_3_PI_8 ) {                             //---------------*--------------> x
                pos.y = source_point.y;                                         //             * | *
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

void MovePoint(Point *point, SDL_FPoint pos, bool shift_pressed, bool ctrl_pressed, bool alt_pressed) {
        

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
                pos = _GetStraightPos(pos, point->next->cords);
        } else if ( alt_pressed && point->prev ) {
                pos = _GetStraightPos(pos, point->prev->cords);
        } else if ( shift_pressed ) {
                pos = _GetStraightPos(pos, start_point);
        } else {
                pos = Vector_Sum(pos, mouse_point);
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


bool _TouchLine(SDL_FPoint P1, SDL_FPoint P2, SDL_FPoint M, float line_r, float point_r) {
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
                _TouchLine(point->cords, point->next->cords, mouse_pos, fixed_line_radius, fixed_radius)
        ) {
                if ( mouse_pressed && prev_mouse_state == 0 ) {
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


bool CheckPoint(PArray *points, Point *point, SDL_FPoint mouse_pos, bool mouse_pressed, bool prev_mouse_state, float fixed_radius) {
        PState new = PSTATE_NONE_STATE;

        if ( _PointUnderMouse(point->cords, mouse_pos, fixed_radius) ) {
                if ( mouse_pressed && prev_mouse_state == 0 ) {
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

bool _TouchVector(SDL_FPoint _Start, SDL_FPoint _Mouse_pos, float angle, float width, float len) {
        SDL_FPoint _End = Vector_Rotate( (SDL_FPoint){0, len}, angle);
        _End.y = -_End.y;
        _End = Vector_Sum(_Start, _End);
        return _TouchLine(_Start, _End, _Mouse_pos, width, 0);
}


bool CheckVector(PArray *points, Point *point, SDL_FPoint mouse_pos, bool mouse_pressed, bool prev_mouse_state, float fixed_line_radius, float fixed_vector_len ) {
        PState new = PSTATE_NONE_STATE;

        if (    
                _TouchVector(point->cords, mouse_pos, point->angle, fixed_line_radius, fixed_vector_len)
        ) {
                if ( mouse_pressed && prev_mouse_state == 0 ) {
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


void CheckSelectedPoint(  PArray *points, SDL_FPoint mouse_pos, bool mouse_pressed, 
                          bool shift_pressed, bool ctrl_pressed, bool alt_pressed,
                          float fixed_radius, float fixed_line_radius, float fixed_vector_width, float fixed_vector_len ) {
        switch ( points->selected_point->state ) {
                case PSTATE_SELECTED:
                        if ( mouse_pressed ) {
                                MovePoint(points->selected_point, mouse_pos, shift_pressed, ctrl_pressed, alt_pressed);
                        } else if ( _PointUnderMouse(points->selected_point->cords, mouse_pos, fixed_radius) ) {
                                points->selected_point->state = PSTATE_UNDER_MOUSE;
                        } else {
                                points->selected_point->state = PSTATE_NONE_STATE;
                                points->selected_point = NULL;
                        }
                        break;
                case PSTATE_LINE_SELECTED:
                        if ( mouse_pressed == 0 ) {
                                if (    points->selected_point->next && 
                                        _TouchLine(points->selected_point->cords, points->selected_point->next->cords, mouse_pos, fixed_line_radius, fixed_radius) ) 
                                {
                                        points->selected_point->state = PSTATE_LINE_UNDER_MOUSE;
                                } else {
                                        points->selected_point->state = PSTATE_NONE_STATE;
                                        points->selected_point = NULL;
                                }
                        }
                        break;
                case PSTATE_VECTOR_SELECTED:
                        if ( mouse_pressed ) {
                                MoveVector(points->selected_point, mouse_pos);
                        } else if ( 
                                _TouchVector(points->selected_point->cords, mouse_pos, points->selected_point->angle, fixed_vector_width, fixed_vector_len)
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



bool CheckMousePos(PArray *points, SDL_FPoint mouse_pos, SDL_FRect texture_box, bool mouse_pressed, bool prev_mouse_state, bool shift_pressed, bool ctrl_pressed, bool alt_pressed) {
        float fixed_radius = POINT_RADIUS * BOX_HEIGHT / texture_box.h;
        float fixed_line_radius = LINE_RADIUS * BOX_HEIGHT / texture_box.h;
        float fixed_vector_len = DIR_VECTOR_LENGTH * BOX_HEIGHT / texture_box.h;
        float fixed_vector_radius = DIR_VECTOR_WIDTH * BOX_HEIGHT / texture_box.h;

        mouse_pos = CordsToBox(mouse_pos, texture_box);

        if ( points->selected_point ) {
                CheckSelectedPoint(     points, mouse_pos, mouse_pressed, 
                                        shift_pressed, ctrl_pressed, alt_pressed,
                                        fixed_radius, fixed_line_radius, fixed_vector_radius, fixed_vector_len );
                points->changed = 1;
        }
        
        Point *now_point = points->points;
        while ( now_point ) {
                if ( points->selected_point ) {
                        break;
                }

                points->changed |= CheckVector(points, now_point, mouse_pos, mouse_pressed, prev_mouse_state, fixed_vector_radius, fixed_vector_len);

                if ( points->selected_point == NULL )
                        points->changed |= CheckPoint(points, now_point, mouse_pos, mouse_pressed, prev_mouse_state, fixed_radius);

                if ( points->selected_point == NULL && now_point->next ) {
                        points->changed |= CheckLine(points, now_point, mouse_pos, mouse_pressed, prev_mouse_state, fixed_radius, fixed_line_radius);
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







void AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line) {
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "couldn`n allocate memory");
                return;
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

void AddPoint_tostart(PArray *points, SDL_FPoint cords, float *angle) {
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

        if ( angle ) {
                new->angle = *angle;
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
#include "path.h"

#define SGN(X) ( (X) > 0 ? 1 : -1 )
#define COS_PI_8 (0.92387953251128675612818318939679f)   // cos(PI/8)
#define COS_3_PI_8 (0.3826834323650897717284599840304f)  // cos(PI*3/8)

// --------------------- helper functions ---------------------
bool _PointUnderMouse(SDL_FPoint cords, SDL_FPoint mouse_cords, float r) {
        return Vector_SqAbs(Vector_Sub(cords, mouse_cords)) < r * r;
}



// --------------------- main functions ---------------------
static SDL_FPoint start_point;
static float start_angle;
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
                        
                        mouse_point = Vector_Sub(start_point, mouse_pos);
                } else {
                        new = PSTATE_UNDER_MOUSE;
                }
                start_point = point->cords;
                start_angle = point->angle;
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
        point->angle = Vector_Angle(Dv);
        // printf("{%f;%f} %f\n", Dv.x, Dv.y, point->angle);

        if ( _Parameters->alt_pressed && _Parameters->ctrl_pressed && point->next ) {
                Dv = Vector_Sub(point->next->cords, point->cords);
                float angle2 = Vector_Angle(Dv);
                point->angle = round( (point->angle - angle2 ) / M_PI_4 ) * M_PI_4 + angle2;
        } else if ( _Parameters->shift_pressed ) {
                point->angle = round(point->angle / M_PI_4 ) * M_PI_4;
        } else if ( _Parameters->alt_pressed && point->prev ) {
                Dv = Vector_Sub(point->cords, point->prev->cords);
                float angle2 = Vector_Angle(Dv);
                point->angle = round( (point->angle - angle2 ) / M_PI_4 ) * M_PI_4 + angle2;
        } else if ( _Parameters->ctrl_pressed && point->prev && point->next ) {
                Dv = Vector_Sub(point->prev->cords, point->next->cords);
                float angle2 = Vector_Angle(Dv);
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
                Point *selected = points->selected_point;
                PState selected_state = selected->state;
                SDL_FPoint selectted_cords = start_point;
                float selected_angle = start_angle;
                CheckSelectedPoint( points, mouse_pos, _Parameters );
                points->changed = 1;

                if ( selected_state != selected->state && ( selected_state == PSTATE_SELECTED || selected_state == PSTATE_VECTOR_SELECTED ) ) {
                        if ( selected->cords.x != selectted_cords.x || selected->cords.y != selectted_cords.y || selected->angle != selected_angle ) {
                                PathAddAction(MOVE_POINT, selected, selectted_cords, selected_angle);
                        }
                }
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







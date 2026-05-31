#include "path.h"


uint32_t last_id = 1;

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



Point *AddPoint(PArray *points, SDL_FPoint cords, float *angle, Point *line, UserField *_Fields, Parametrs *_Parameters) {
        if ( points == NULL || _Parameters == NULL ) {
                return NULL;
        }
        
        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "malloc failed");
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
                last_id,
                NULL,
                NULL
        };
        last_id++;
        
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
                new->angle = Vector_Angle(Dv);
                
                goto result;
        }

        // add point to end of the PArray
        Point *now = points->points;
        if ( now == NULL ) {
                points->points = new;
                if ( angle ) {
                        new->angle = *angle;
                }
                points->count++;
                goto result;
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
                new->angle = Vector_Angle(Dv);
        }
        points->count++;

        result:
        PathAddAction(ADD_POINT, new, new->cords, new->angle);
        return new;
}

Point *AddPoint_tostart(PArray *points, SDL_FPoint cords, float angle, UserField *_Fields, Parametrs * _Parameters) {
        if ( points == NULL || _Parameters == NULL ) {
                return NULL;
        }

        Point *new = malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("AddPoint", "malloc failed");
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
                last_id,
                NULL,
                NULL
        }; 
        last_id++;

        new->next = points->points;
        points->points = new;
        if ( new->next ) {
                new->next->prev = new;
        }

        new->angle = angle;
        points->count++;

        PathAddAction(ADD_POINT_TO_START, new, new->cords, new->angle);
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

        PathAddAction(DEL_POINT, point, point->cords, point->angle);
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
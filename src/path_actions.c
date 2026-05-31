#include "path.h"

ACTION *actions = NULL;
ACTION *last_action = NULL;


bool _RestorePoint(PArray *_Points) {
        Point *new = (Point*)malloc(sizeof(Point));
        if ( new == NULL ) {
                LogError("PathUndo", "malloc failed");
                return 0;
        }

        *new = (Point){
                last_action->start_cords,
                last_action->start_angle,
                PSTATE_NONE_STATE,
                NULL,
                last_action->id,
                NULL,
                NULL
        };

        if ( last_action->prev_id == 0 || _Points->points == NULL ) {
                new->next = _Points->points;
                _Points->points = new;
                return 1;
        }

        Point *point = _Points->points;
        while ( point != NULL && point->id != last_action->prev_id ) {
                point = point->next;
        }

        if ( point == NULL ) {
                free(new);
                LogWarn("_RestorePoint", "No point founf with id %u", last_action->prev_id);
                return 0;
        }


        if ( point->next ) {
                new->next = point->next;
                point->next->prev = new;
        }
        new->prev = point;
        point->next = new;

        return 1;
}

bool _DelPoint(PArray *_Points) {
        if ( _Points->points == NULL || last_action->id == 0 ) {
                LogWarn("_DelPoint", "No point founf with id %u", last_action->id);
                return 0;
        }

        Point *point = _Points->points;
        while ( point != NULL && point->id != last_action->id ) {
                point = point->next;
        }

        if  ( point == NULL ) {
                return 0;
        }

        if ( point->prev ) {
                point->prev->next = point->next;
        } else {
                _Points->points = point->next;
        }

        if ( point->next ) {
                point->next->prev = point->prev;
        }
        
        FreePoint(point);
        _Points->count--;

        return 1;
}

bool _MoveBack(PArray *_Points) {
       if ( _Points->points == NULL || last_action->id == 0 ) {
                LogWarn("_MoveBack", "No point founf with id %u", last_action->id);
                return 0;
        }

        Point *point = _Points->points;
        while ( point != NULL && point->id != last_action->id ) {
                point = point->next;
        }
        
        if  ( point == NULL ) {
                return 0;
        }

        point->cords = last_action->start_cords;
        point->angle = last_action->start_angle;
        return 1;
}

bool _MoveForward(PArray *_Points) {
       if ( _Points->points == NULL || last_action->id == 0 ) {
                LogWarn("_MoveForward", "No point founf with id %u", last_action->id);
                return 0;
        }

        Point *point = _Points->points;
        while ( point != NULL && point->id != last_action->id ) {
                point = point->next;
        }
        
        if  ( point == NULL ) {
                return 0;
        }

        point->cords = last_action->res_cords;
        point->angle = last_action->res_angle;
        return 1;
}


bool PathUndo(PArray *_Points) {
        if ( last_action == NULL || _Points == NULL ) {
                return 0;
        }
        
        bool res;
        switch ( last_action->type ) {
                case DEL_POINT:
                        res = _RestorePoint(_Points);
                        LogDebug("_RestorePoint", "restore point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                case ADD_POINT:
                        res = _DelPoint(_Points);
                        LogDebug("_DelPoint", "delete point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;
                
                case ADD_POINT_TO_START:
                        res = _DelPoint(_Points);
                        LogDebug("_DelPoint", "delete point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                case MOVE_POINT:
                        res = _MoveBack(_Points);
                        LogDebug("_MoveBack", "move point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                default:
                        res = 0;
                        break;
                        
        }

        last_action = last_action->prev;
        return res;
}


bool PathRedo(PArray *_Points) {
        if ( _Points == NULL ) {
                return 0;
        }
        
        if ( last_action == NULL ) {
                if ( actions != NULL ) {
                        last_action = actions;
                } else {
                        return 0;
                }
        }  else {
                if ( last_action->next ) {
                        last_action = last_action->next;
                } else {
                        return 0;
                }
        }
        
        bool res;
        switch ( last_action->type ) {
                case DEL_POINT:
                        res = _DelPoint(_Points);
                        LogDebug("_DelPoint", "delete point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                case ADD_POINT:
                        res = _RestorePoint(_Points);
                        LogDebug("_RestorePoint", "restore point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;
                
                case ADD_POINT_TO_START:
                        res = _RestorePoint(_Points);
                        LogDebug("_RestorePoint", "restore point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                case MOVE_POINT:
                        res = _MoveForward(_Points);
                        LogDebug("_MoveForward", "move point %u->%u. reult: %i\n", last_action->prev_id, last_action->id, res);
                        break;

                default:
                        res = 0;
                        break;
                        
        }

        return res;
}


bool PathAddAction(ACTION_TYPE _Type, Point *_Point, SDL_FPoint _Start_cord, double _Start_ang) {
        if ( _Point == NULL ) {
                return 0;
        }

        ACTION *new = (ACTION*)malloc(sizeof(ACTION));
        if ( new == NULL ) {
                LogError("AddPoint", "malloc failed");
                return 0;
        }
        new->type = _Type;

        new->id = _Point->id;
        if ( _Point->prev ) {
                new->prev_id = _Point->prev->id;
        } else {
                new->prev_id = 0;
        }

        new->start_cords = _Start_cord;
        new->start_angle = _Start_ang;
        new->res_cords = _Point->cords;
        new->res_angle = _Point->angle;

        new->next = NULL;
        new->prev = last_action;

        if ( last_action ) {
                PathFreeAction(last_action->next);
                last_action->next = new;
        } else {
                PathFreeAction(actions);
                actions = new;
        }
        last_action = new;
        LogDebug("PathAddAction", "Added action %u->%u", new->prev_id, new->id);
        return 1;
}


void PathClearActions() {
        PathFreeAction(actions);
}


void PathFreeAction(ACTION *_Action) {
        while ( _Action ) {
                ACTION *tmp = _Action->next;
                free(_Action);
                _Action = tmp;
        }        
}






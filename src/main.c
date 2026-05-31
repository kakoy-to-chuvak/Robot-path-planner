#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>


#include "parametrs.h"
#include "label.h"
#include "App.h"
#include "logs.h"
#include "drawing.h"
#include "vectors.h"
#include "path.h"
#include "contextmenu.h"
#include "filesave.h"
#include "crossplatform.h"



APP *app;
LABEL *point_text;

CONTEXTMENU *contextmenu;
LABEL *menu_labels[3];
MENU_BUTTON *menu_buttons[3];

SDL_Texture *background_texture = NULL;
SDL_Texture *point_texture = NULL;

Parametrs parametrs;

FileSaveArgs file_save_args;


PArray points = {
        1,
        0,
        NULL,
        NULL,
        "",
};

struct menu_args {
        Point *point;
        SDL_FPoint cords;
};


// contextmenu buttons functions
void *Menu_AddPoint(void *contextmenu, void *args_vpointer) {
        // get args
        struct menu_args args = *((struct menu_args*)args_vpointer);
        SDL_FPoint cords = WindowCordsToBox(args.cords, &parametrs);
        
        // Add point
        AddPoint(&points, cords, NULL, args.point, NULL, &parametrs);

        // useless return
        return contextmenu;
}

void *Menu_DelPoint(void *contextmenu, void *args_vpointer) {
        // get args
        struct menu_args args = *((struct menu_args*)args_vpointer);
        if ( args.point ) {
                // Del point
                DelPoint(&points, args.point);
        }

        // useless return
        return contextmenu;
}

void *Menu_AddPointToStart(void *contextmenu, void *args_vpointer) {
        // get args
        struct menu_args args = *((struct menu_args*)args_vpointer);

        // window cords to box cords
        SDL_FPoint cords = WindowCordsToBox(args.cords, &parametrs);

        // Add point
        AddPoint_tostart(&points, cords, 0, NULL, &parametrs);

        // useless return
        return contextmenu;
}



int render(APP *app) {
        LogTrace("render", "render");
        
        // reseting render
        SDL_SetRenderDrawColor(app->Renderer, 0, 0, 0, 255);
        SDL_RenderClear(app->Renderer);

        // render background
        SDL_RenderTexture(app->Renderer, background_texture, NULL, &parametrs.texture_box);

        // render points, lines and vectors
        RenderPath(app->Renderer, point_texture, &points, point_text, &parametrs);

        // render context contextmenu
        ContextMenu_Render(contextmenu);

        // present render
        SDL_RenderPresent(app->Renderer);
        return 1;
}


int setup(APP *app) {
        LogDebug("setup", "entering setup");

        // load point texture
        LogDebug("setup", "IMG_Load: loading [images/point.png] to [tmp_surf]" );
        SDL_Surface *tmp_surf = IMG_Load("images/point.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        // convert point texture
        LogDebug("setup", "SDL_CreateTextureFromSurface: create [point_texture]" );
        point_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == point_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }
        
        // load background texture
        LogDebug("setup", "IMG_Load: loading [images/ground.png] to [tmp_surf]" );
        tmp_surf = IMG_Load("images/ground.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        // convert background texture
        LogDebug("setup", "SDL_CreateTextureFromSurface: create [background_texture]" );
        background_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == background_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }

        // create label for displaing point cords
        LogDebug("setup", "Label_New: create label [point_text]" );
        point_text = Label_New(app->Renderer, "fonts/" POINT_CORDS_FONT, "(0, 0)", TEXT_SIZE, TEXT_COLOR_Black, LABEL_PARAM_BORDER, TEXT_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }


        // ==== Menu ====
        // create labels for contextmenu buttons
        LogDebug("setup", "Label_New: creatng contextmenu label [0]" );
        menu_labels[0] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Add point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }
        
        LogDebug("setup", "Label_New: creatng contextmenu label [1]" );
        menu_labels[1] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Add point to start", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        LogDebug("setup", "Label_New: creatng contextmenu label [2]" );
        menu_labels[2] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Delete point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        // create context contextmenu
        LogDebug("setup", "Menu_New: create contextmenu: [contextmenu]" );
        contextmenu = ContextMenu_New(app->Renderer, SDL_PIXELFORMAT_RGBA32,
                MENU_BG, 5, 0, MENU_BORDER_COLOR);
        if ( NULL == contextmenu ) {
                LogError("setup", "Menu_New failed");
                return 0;
        }

        // create contextmenu buttons
        LogDebug("setup", "Menu_SetupButtons & Menu_SetButton: setup contextmenu buttons");
        ContextMenu_SetupButtons(contextmenu, 6, 170, 30, MENU_BG, MENU_TRIGGER_COLOR, 4, 4, 5, 6);
        menu_buttons[2] = ContextMenu_SetButton(contextmenu, 2, menu_labels[2], 0, 1, Menu_DelPoint); 
        menu_buttons[1] = ContextMenu_SetButton(contextmenu, 1, menu_labels[1], 0, 1, Menu_AddPointToStart); 
        menu_buttons[0] = ContextMenu_SetButton(contextmenu, 0, menu_labels[0], 0, 1, Menu_AddPoint); 
        
        // fix render
        SDL_SetRenderDrawBlendMode(app->Renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(point_texture, 0, 0, 0);

        // first render
        LogDebug("setup", "first render");
        render(app);

        LogDebug("setup", "Exit setup");

        return 1;
}





int Tick(APP *app) {
        // variables
        SDL_Event event;
        static SDL_FPoint mouse_pos = {
                0, 0
        };

        static struct menu_args args = {
                NULL,
                {0, 0}
        };


        // events
        while ( SDL_PollEvent(&event) ) {
                switch (event.type) {
                        case SDL_EVENT_QUIT:
                                // close app
                                app->is_running = 0;
                                return 0;
                                break;
                        case SDL_EVENT_MOUSE_MOTION:
                                mouse_pos.x = event.motion.x;
                                mouse_pos.y = event.motion.y;
                                break;
                        case SDL_EVENT_MOUSE_BUTTON_DOWN:
                                // mouse buttons state
                                if ( event.button.button == SDL_BUTTON_LEFT ) {
                                        parametrs.lmb_pressed = 1;
                                } else if ( event.button.button == SDL_BUTTON_RIGHT ) {
                                        parametrs.rmb_pressed = 1;
                                }
                                break;
                        case SDL_EVENT_MOUSE_BUTTON_UP:
                                // mouse buttons state
                                if ( event.button.button == SDL_BUTTON_LEFT ) {
                                        parametrs.lmb_pressed = 0;
                                } else if ( event.button.button == SDL_BUTTON_RIGHT ) { 
                                        parametrs.rmb_pressed = 0;
                                }
                                break; 
                        case SDL_EVENT_KEY_DOWN: 
                                LogTrace("Tick", "Key down. Scancode: %i  Char: '%c'", event.key.scancode, event.key.key);
                                switch (event.key.scancode) {
                                        case SDL_SCANCODE_ESCAPE:
                                                // close app
                                                app->is_running = 0;
                                                return 0;
                                                break;
                                        // keys state
                                        case SDL_SCANCODE_LSHIFT:
                                                parametrs.shift_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_LCTRL:
                                                parametrs.ctrl_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_LALT:
                                                parametrs.alt_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_S:
                                                // save file if "ctrl" pressed (ctrl+s)
                                                if ( parametrs.ctrl_pressed == 0 ) {
                                                        break;
                                                }

                                                LogDebug("Tick", "Saving file");
                                                // open file dialog if no file selected or shift pressed (ctrl+shift+s)
                                                if ( parametrs.shift_pressed == 1 || *points.file_name == '\0' ) {
                                                        file_save_args = (FileSaveArgs){
                                                                &points,
                                                                &parametrs
                                                        };
                                                        ShowSaveFIleDialog(NULL, points.file_name, &file_save_args);
                                                } else {
                                                        SavePoints(&points);
                                                }
                                                
                                                break;
                                        case SDL_SCANCODE_O:
                                                // open file if "ctrl" pressed (ctrl+o)
                                                if ( parametrs.ctrl_pressed == 0 ) {
                                                        break;
                                                }

                                                file_save_args = (FileSaveArgs){
                                                        &points,
                                                        &parametrs
                                                };

                                                LogDebug("Tick", "Opening file");
                                                ShowOpenFIleDialog(NULL, points.file_name, &file_save_args);

                                                break;
                                        case SDL_SCANCODE_F11:
                                                // fullscreen mode
                                                if ( SDL_GetWindowFlags(app->Window) & SDL_WINDOW_MAXIMIZED ) {
                                                        SDL_RestoreWindow(app->Window);
                                                } else {
                                                        SDL_MaximizeWindow(app->Window);
                                                }

                                                break;
                                        
                                        case SDL_SCANCODE_Z:
                                                if ( parametrs.ctrl_pressed ) {
                                                        if ( parametrs.shift_pressed ) {
                                                                PathRedo(&points);
                                                        } else {
                                                                PathUndo(&points);
                                                        }
                                                        points.changed = 1;
                                                }
                                                break;

                                        default:
                                                break;
                                }
                                break;
                        case SDL_EVENT_KEY_UP:
                                // keys state
                                switch ( event.key.scancode ) {
                                        case SDL_SCANCODE_LSHIFT:
                                                parametrs.shift_pressed = 0;
                                                break;
                                        case SDL_SCANCODE_LCTRL:
                                                parametrs.ctrl_pressed = 0;
                                                break;
                                        case SDL_SCANCODE_LALT:
                                                parametrs.alt_pressed = 0;
                                                break;
                                        default:
                                                break;
                                }
                                break;
                        case SDL_EVENT_WINDOW_RESIZED:
                                // fix values for right handle
                                ParametrsFixValues(&parametrs, app->Window);
                                points.changed = 1;
                                break;
                        default:
                                break;
                }
        }
        
        // check points changes 
        CheckMousePos(&points, mouse_pos, &parametrs );

        bool lmb_clicked = parametrs.lmb_pressed == 0 && parametrs.prev_lmb_state;
        bool rmb_clicked = parametrs.rmb_pressed == 0 && parametrs.prev_rmb_state;
        
        // check contextmenu activation
        if ( rmb_clicked && ( contextmenu->active == 0 || ContextMenu_MouseOut(contextmenu, mouse_pos.x, mouse_pos.y) ) ) {
                args.cords = mouse_pos;

                // activate contextmenu
                ContextMenu_Move(contextmenu, mouse_pos.x, mouse_pos.y, parametrs.window_w, parametrs.window_h);
                contextmenu->active = 1;
                points.changed = 1;

                args.point = points.selected_point;

                // make label active/inactive
                if ( args.point == NULL ) {
                        menu_buttons[2]->active = 0;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_Grey);
                } else {
                        menu_buttons[2]->active = 1;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_White);
                }
        }

        // check contextmenu changes
        points.changed |= ContextMenu_CheckUpdate(contextmenu, mouse_pos.x, mouse_pos.y, lmb_clicked | rmb_clicked, &args);

        // render if something has changed
        if ( points.changed ) {
                render(app);
                points.changed = 0;
        }
        
        // get previous state
        parametrs.prev_lmb_state = parametrs.lmb_pressed;
        parametrs.prev_rmb_state = parametrs.rmb_pressed;

        return 1;
}



int main( int argc, char *argv[] ) {
        char user_path[MAX_PATH] = "";
        if ( getcwd(user_path, sizeof(user_path)) == NULL ) {
                return -1;
        }

        char *last_slash = strrchr(argv[0], '\\');
        if ( last_slash == NULL ) {
                last_slash = strrchr(argv[0], '/');
                if ( last_slash == NULL ) {
                        return -1;
                }
        }
        *last_slash = '\0';
        
        CRP_chdir(argv[0]);


        // setup logs
        Logs_SetFile("logs.log");
        Logs_SetLogLevel(LOG_LEVEL_NOTICE       );
        Logs_EnableColors(0);
        

        // init SDL3
        if ( 0==SDL_Init(SDL_INIT_FLAGS) ) {
                LogError("main", "SDL_Init failed: %s", SDL_GetError());
                return 0;
        }

        // init SDL_ttf
        if ( 0==TTF_Init() ) {
                LogError("setup/TTF_Init", "TTF_Init failed: %s", SDL_GetError());
                return 0;
        }   

        // create new app
        app = AppNew("Планировщик маршрута", 600, 400, SDL_WINDOW_RESIZABLE, NULL);
        if ( NULL==app ) {
                LogError("main", "AppNew failed");
                goto app_quit;
        }

        // init parametrs
        ParametrsInit(&parametrs, app->Window);

        // setup everything
        if ( 0==setup(app) ) {
                LogError("main", "setup failed");
                goto app_quit;
        }

        CRP_chdir(user_path);

        // opening file from second argument
        if ( argc == 2 ) {
                strcpy_s(points.file_name, MAX_PATH, argv[1]);
                LoadPoints(&points, &parametrs);
        } 

        // init app
        AppSetTick(app, Tick);
        AppSetTps(app, TPS);
        AppMainloop(app);
        
        app_quit:
        // free objects and quit app
        FreePoints(&points);
        PathClearActions();
        Label_Free(point_text);

        ContextMenu_Free(contextmenu);
        Label_Free(menu_labels[0]);
        Label_Free(menu_labels[1]);
        Label_Free(menu_labels[2]);

        SDL_DestroyTexture(point_texture);
        SDL_DestroyTexture(background_texture);
        AppQuit(app);
        TTF_Quit();
        SDL_Quit();
        return 0;
}
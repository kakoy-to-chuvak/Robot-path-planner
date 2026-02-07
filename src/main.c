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
#include "menu.h"
#include "filesave.h"



APP *app;
LABEL *point_text;

MENU *menu;
LABEL *menu_labels[3];
MENU_BUTTON *menu_buttons[3];

SDL_Texture *background_texture = NULL;
SDL_Texture *point_texture = NULL;

Parametrs parametrs;

FileSaveArgs file_save_args;


PArray points = {
        1,
        NULL,
        NULL,
        "",
        FILE_FORMAT_UNDEFINED,
};

struct menu_args {
        Point *point;
        SDL_FPoint cords;
};


// menu buttons functions
void *Menu_AddPoint(void *menu, void *args_vpointer) {
        struct menu_args args = *((struct menu_args*)args_vpointer);
        SDL_FPoint cords = args.cords;
        
        cords.x -= parametrs.texture_box.x;
        cords.y -= parametrs.texture_box.y;

        cords.x *= parametrs.box_width / parametrs.texture_box.w;
        cords.y *= parametrs.box_height / parametrs.texture_box.h;

        AddPoint(&points, cords, NULL, args.point, &parametrs);

        return menu;
}

void *Menu_DelPoint(void *menu, void *args_vpointer) {
        struct menu_args args = *((struct menu_args*)args_vpointer);
        if ( args.point ) {
                DelPoint(&points, args.point);
        }

        return menu;
}

void *Menu_AddPointToStart(void *menu, void *args_vpointer) {
        struct menu_args args = *((struct menu_args*)args_vpointer);

        SDL_FPoint cords = args.cords;
        
        cords.x -= parametrs.texture_box.x;
        cords.y -= parametrs.texture_box.y;

        cords.x *= parametrs.box_width / parametrs.texture_box.w;
        cords.y *= parametrs.box_height / parametrs.texture_box.h;

        AddPoint_tostart(&points, cords, 0, &parametrs);

        return menu;
}



int render(APP *app) {
        LogTrace("render", "render");
        
        SDL_SetRenderDrawColor(app->Renderer, 0, 0, 0, 255);
        SDL_RenderClear(app->Renderer);

        SDL_RenderTexture(app->Renderer, background_texture, NULL, &parametrs.texture_box);

        RenderPath(app->Renderer, point_texture, &points, point_text, &parametrs);

        Menu_Render(menu);
        
        SDL_RenderPresent(app->Renderer);
        return 1;
}


int setup(APP *app) {
        LogDebug("setup", "IMG_Load: loading [images/point.png] to [tmp_surf]" );
        SDL_Surface *tmp_surf = IMG_Load("images/point.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        
        LogDebug("setup", "SDL_CreateTextureFromSurface: creating [point_texture]" );
        point_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == point_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }
        
        LogDebug("setup", "IMG_Load: loading [images/ground.png] to [tmp_surf]" );
        tmp_surf = IMG_Load("images/ground.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        LogDebug("setup", "SDL_CreateTextureFromSurface: creating [background_texture]" );
        background_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == background_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }

        LogDebug("setup", "Label_New: creating label [point_text]" );
        point_text = Label_New(app->Renderer, "fonts/" POINT_CORDS_FONT, "(0, 0)", TEXT_SIZE, TEXT_COLOR_Black, LABEL_PARAM_BORDER, TEXT_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }


        // Menu
        LogDebug("setup", "Label_New: creatng menu label [0]" );
        menu_labels[0] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Add point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }
        
        LogDebug("setup", "Label_New: creatng menu label [1]" );
        menu_labels[1] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Add point to start", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        LogDebug("setup", "Label_New: creatng menu label [2]" );
        menu_labels[2] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Delete point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }


        LogDebug("setup", "Menu_New: creating menu: [menu]" );
        menu = Menu_New(app->Renderer, SDL_PIXELFORMAT_RGBA32,
                MENU_BG, 5, 0, MENU_BORDER_COLOR);
        if ( NULL == menu ) {
                LogError("setup", "Menu_New failed");
                return 0;
        }

        LogDebug("setup", "Menu_SetupButtons & Menu_SetButton: setup menu buttons");
        Menu_SetupButtons(menu, 6, 170, 30, MENU_BG, MENU_TRIGGER_COLOR, 4, 4, 5, 6);
        menu_buttons[2] = Menu_SetButton(menu, 2, menu_labels[2], 0, 1, Menu_DelPoint); 
        menu_buttons[1] = Menu_SetButton(menu, 1, menu_labels[1], 0, 1, Menu_AddPointToStart); 
        menu_buttons[0] = Menu_SetButton(menu, 0, menu_labels[0], 0, 1, Menu_AddPoint); 
        

        SDL_SetRenderDrawBlendMode(app->Renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(point_texture, 0, 0, 0);

        LogDebug("setup", "first render");
        render(app);

        LogDebug("setup", "deleting temporary variables");
        SDL_DestroySurface(tmp_surf);

        LogDebug("setup", "end setup");

        return 1;
}





int Tick(APP *app) {
        SDL_Event event;
        static SDL_FPoint mouse_pos = {
                0, 0
        };

        while ( SDL_PollEvent(&event) ) {
                switch (event.type) {
                        case SDL_EVENT_QUIT:
                                app->is_running = 0;
                                return 0;
                                break;
                        case SDL_EVENT_MOUSE_MOTION:
                                mouse_pos.x = event.motion.x;
                                mouse_pos.y = event.motion.y;
                                break;
                        case SDL_EVENT_MOUSE_BUTTON_DOWN:
                                if ( event.button.button == SDL_BUTTON_LEFT ) {
                                        parametrs.lmb_pressed = 1;
                                } else if ( event.button.button == SDL_BUTTON_RIGHT ) {
                                        parametrs.rmb_pressed = 1;
                                }
                                break;
                        case SDL_EVENT_MOUSE_BUTTON_UP:
                                if ( event.button.button == SDL_BUTTON_LEFT ) {
                                        parametrs.lmb_pressed = 0;
                                } else if ( event.button.button == SDL_BUTTON_RIGHT ) { 
                                        parametrs.rmb_pressed = 0;
                                }
                                break; 
                        case SDL_EVENT_KEY_DOWN: 
                                switch (event.key.scancode) {
                                        case SDL_SCANCODE_ESCAPE:
                                                app->is_running = 0;
                                                return 0;
                                                break;
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
                                                if ( parametrs.ctrl_pressed == 0 )
                                                        break;

                                                if ( parametrs.shift_pressed == 1 || *points.file_name == 0 ) {
                                                        file_save_args = (FileSaveArgs){
                                                                &points,
                                                                &parametrs
                                                        };
                                                        ShowSaveFIleDialog(NULL, NULL, &file_save_args);
                                                } else {
                                                        SavePoints(&points);
                                                }
                                                
                                                break;
                                        case SDL_SCANCODE_O:
                                                if ( parametrs.ctrl_pressed == 0 )
                                                        break;

                                                file_save_args = (FileSaveArgs){
                                                        &points,
                                                        &parametrs
                                                };
                                                ShowOpenFIleDialog(NULL, NULL, &file_save_args);
                                                break;
                                        case SDL_SCANCODE_F11:
                                                if ( SDL_GetWindowFlags(app->Window) & SDL_WINDOW_MAXIMIZED )
                                                        SDL_RestoreWindow(app->Window);
                                                else
                                                        SDL_MaximizeWindow(app->Window);

                                                break;

                                        default:
                                                break;
                                }
                                break;
                        case SDL_EVENT_KEY_UP:
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
                                ParametrsFixValues(&parametrs, app->Window);
                                points.changed = 1;
                                break;
                        default:
                                break;
                }
        }
        

        static bool prev_lmb_state = 0;
        bool lmb_clicked = parametrs.lmb_pressed && prev_lmb_state == 0;

        static bool prev_rmb_state = 0;
        bool rmb_clicked = parametrs.rmb_pressed && prev_rmb_state == 0;

        static struct menu_args args = {
                NULL,
                {0, 0}
        };

        
        CheckMousePos(&points, mouse_pos, &parametrs );
        
        if ( rmb_clicked && prev_rmb_state == 0 && ( menu->active == 0 || Menu_MouseOut(menu, mouse_pos.x, mouse_pos.y) ) ) {
                args.cords = mouse_pos;

                Menu_Move(menu, mouse_pos.x, mouse_pos.y, parametrs.window_w, parametrs.window_h);
                menu->active = 1;
                points.changed = 1;

                args.point = points.selected_point;

                if ( args.point == NULL ) {
                        menu_buttons[2]->active = 0;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_Grey);
                } else {
                        menu_buttons[2]->active = 1;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_White);
                }
        }

        points.changed |= Menu_CheckUpdate(menu, mouse_pos.x, mouse_pos.y, lmb_clicked | rmb_clicked, &args);

        parametrs.prev_lmb_state = parametrs.lmb_pressed;
        parametrs.prev_rmb_state = parametrs.rmb_pressed;

        if ( points.changed ) {
                render(app);
                points.changed = 0;
        }

        return 1;
}



int main( int argc, char *argv[] ) {
        if ( argc == 0 ) {
                return -1;
        }

        char *t = strrchr(argv[0], '\\');
        if ( t == NULL ) {
                t = strrchr(argv[0], '/');
                if ( t == NULL ) {
                        return -1;
                }
        }

        *t = '\0';
        if ( CRP_chdir(argv[0]) ) {
                return -1;
        }

        Logs_SetFile("logs.log");
        Logs_SetLogLevel(LOG_LEVEL_NOTICE);
        Logs_EnableColors(0);

        if ( 0==SDL_Init(SDL_INIT_FLAGS) ) {
                LogError("main", "SDL_Init failed: %s", SDL_GetError());
                return 0;
        }

        if ( 0==TTF_Init() ) {
                LogError("setup/TTF_Init", "TTF_Init failed: %s", SDL_GetError());
                return 0;
        }   

        app = AppNew("Планировщик маршрута", 600, 400, SDL_WINDOW_RESIZABLE, NULL);
        if ( NULL==app ) {
                LogError("main", "AppNew failed");
                goto app_quit;
        }

        ParametrsInit(&parametrs, app->Window);

        if ( 0==setup(app) ) {
                LogError("main", "setup failed");
                goto app_quit;
        }

        if ( argc == 2 ) {
                strcpy_s(points.file_name, MAX_PATH, argv[1]);
                points.format = DefineFileFormat(argv[1]);
                LoadPoints(&points, &parametrs);
        } 


        AppSetTick(app, Tick);
        AppSetTps(app, TPS);
        AppMainloop(app);

        app_quit:
        FreePoints(&points);
        
        Label_Free(point_text);

        Menu_Free(menu);
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
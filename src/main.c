#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>


#include "include.h"
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

static int window_w = 0;
static int window_h = 0;

static SDL_FRect background_texture_rect;

bool lmb_pressed = 0;
bool rmb_pressed = 0;
bool shift_pressed = 0;
bool ctrl_pressed = 0;
bool alt_pressed = 0;


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
        
        cords.x -= background_texture_rect.x;
        cords.y -= background_texture_rect.y;

        cords.x *= BOX_WIDTH / background_texture_rect.w;
        cords.y *= BOX_HEIGHT / background_texture_rect.h;

        AddPoint(&points, cords, NULL, args.point);

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
        
        cords.x -= background_texture_rect.x;
        cords.y -= background_texture_rect.y;

        cords.x *= BOX_WIDTH / background_texture_rect.w;
        cords.y *= BOX_HEIGHT / background_texture_rect.h;

        AddPoint_tostart(&points, cords, NULL);

        return menu;
}



int render(APP *app) {
        LogTrace("render", "render");
        
        SDL_SetRenderDrawColor(app->Renderer, 0, 0, 0, 255);
        SDL_RenderClear(app->Renderer);

        SDL_RenderTexture(app->Renderer, background_texture, NULL, &background_texture_rect);

        RenderPath(app->Renderer, point_texture, &points, point_text, background_texture_rect);

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
        menu_labels[1] = Label_New(app->Renderer, "fonts/" MENU_TEXT_FONT, "Add to start", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
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
        Menu_SetupButtons(menu, 6, 200, 30, MENU_BG, MENU_TRIGGER_COLOR, 4, 4, 5, 6);
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
                                if ( event.button.button == 1 ) {
                                        lmb_pressed = 1;
                                } else if ( event.button.button == 3 ) {
                                        rmb_pressed = 1;
                                }
                                break;
                        case SDL_EVENT_MOUSE_BUTTON_UP:
                                if ( event.button.button == 1 ) {
                                        lmb_pressed = 0;
                                } else if ( event.button.button == 3 ) { 
                                        rmb_pressed = 0;
                                }
                                break; 
                        case SDL_EVENT_KEY_DOWN: 
                                switch (event.key.scancode) {
                                        case SDL_SCANCODE_ESCAPE:
                                                app->is_running = 0;
                                                return 0;
                                                break;
                                        case SDL_SCANCODE_LSHIFT:
                                                shift_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_LCTRL:
                                                ctrl_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_LALT:
                                                alt_pressed = 1;
                                                break;
                                        case SDL_SCANCODE_S:
                                                if ( ctrl_pressed == 0 )
                                                        break;

                                                if ( shift_pressed == 1 || *points.file_name == 0 )
                                                        ShowSaveFIleDialog(NULL, NULL, &points);
                                                else
                                                        SavePoints(&points);
                                                
                                                break;
                                        case SDL_SCANCODE_O:
                                                if ( ctrl_pressed == 0 )
                                                        break;

                                                ShowOpenFIleDialog(NULL, NULL, &points);
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
                                                shift_pressed = 0;
                                                break;
                                        case SDL_SCANCODE_LCTRL:
                                                ctrl_pressed = 0;
                                                break;
                                        case SDL_SCANCODE_LALT:
                                                alt_pressed = 0;
                                                break;
                                        default:
                                                break;
                                }
                                break;
                        case SDL_EVENT_WINDOW_RESIZED:
                                SDL_GetWindowSize(app->Window, &window_w, &window_h);
                                float k = BOX_WIDTH / BOX_HEIGHT;
                                
                                // resize background texture
                                if ( ((float)window_w) / ((float)window_h) > k ) {
                                        background_texture_rect.w = window_h * k;
                                        background_texture_rect.h = window_h;
                                        background_texture_rect.y = 0;
                                        background_texture_rect.x = ( window_w - background_texture_rect.w ) / 2;
                                } else {
                                        background_texture_rect.h = window_w / k;
                                        background_texture_rect.w = window_w;
                                        background_texture_rect.x = 0;
                                        background_texture_rect.y = ( window_h - background_texture_rect.h ) / 2;
                                }

                                points.changed = 1;
                                break;
                        default:
                                break;
                }
        }
        

        static bool prev_lmb_state = 0;
        bool lmb_clicked = lmb_pressed && prev_lmb_state == 0;

        static bool prev_rmb_state = 0;
        bool rmb_clicked = rmb_pressed && prev_rmb_state == 0;

        static struct menu_args args = {
                NULL,
                {0, 0}
        };

        
        CheckMousePos(&points, mouse_pos, background_texture_rect, lmb_pressed, prev_lmb_state, shift_pressed, ctrl_pressed, alt_pressed);
        
        if ( rmb_clicked && ( menu->active == 0 || Menu_MouseOut(menu, mouse_pos.x, mouse_pos.y) ) ) {
                args.cords = mouse_pos;

                Menu_Move(menu, mouse_pos.x, mouse_pos.y, window_w, window_h);
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

        prev_lmb_state = lmb_pressed;
        prev_rmb_state = rmb_pressed;

        if ( points.changed ) {
                render(app);
                points.changed = 0;
        }

        return 1;
}



int main() {
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

        app = AppNew("Планировщик маршрута", 0, 0, SDL_WINDOW_RESIZABLE, NULL);
        if ( NULL==app ) {
                LogError("main", "AppNew failed");
                goto app_quit;
        }
        
        if ( 0==setup(app) ) {
                LogError("main", "setup failed");
                goto app_quit;
        }

        window_w = background_texture->w;
        window_h = window_w / BOX_WIDTH * BOX_HEIGHT;
        SDL_SetWindowSize(app->Window, window_w, window_h);

        background_texture_rect.w = window_w;
        background_texture_rect.h = window_h;
        background_texture_rect.x = 0;
        background_texture_rect.y = 0;


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
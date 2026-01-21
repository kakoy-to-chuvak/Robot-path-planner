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
#include "config_parser.h"



APP *app;
LABEL *point_text;

MENU *menu;
LABEL *menu_labels[3];
MENU_BUTTON *menu_buttons[3];

SDL_Texture *background_texture = NULL;
SDL_Texture *point_texture = NULL;

PARSED_CONFIG *config = NULL;

bool lmb_pressed = 0;
bool rmb_pressed = 0;
bool shift_pressed = 0;
bool ctrl_pressed = 0;

bool changes = 1;
PArray points = {
        NULL,
        NULL,
        NULL,
};



// menu buttons functions
void *Menu_AddPoint(void *menu, void *v_point) {
        AddPoint(&points, ((MENU*)menu)->x, ((MENU*)menu)->y, v_point);

        return NULL;
}

void *Menu_DelPoint(void *menu, void *v_point) {
        if ( v_point ) {
                DelPoint(&points, v_point);
        }

        return menu;
}

void *Menu_AddPointToStart(void *menu, void *v_point) {
        AddPoint_tostart(&points, ((MENU*)menu)->x, ((MENU*)menu)->y);

        return v_point;
}



int render(APP *app) {
        if ( changes == 0 )
                return 1;
        changes = 0;
        
        LogTrace("render", "render");
        
        SDL_RenderClear(app->Renderer);

        SDL_RenderTexture(app->Renderer, background_texture, NULL, NULL);

        RenderPath(app->Renderer, point_texture, &points, point_text);

        Menu_Render(menu);
        
        SDL_RenderPresent(app->Renderer);
        return 1;
}


int setup(APP *app) {
        LogDebug("setup", "enter setup");
        LogDebug("setup", "start parsing config");
        config = Config_Parse("config.set");
        char buffer[1024] = "";
        Config_GetElement(config, "qwae", buffer, sizeof(buffer));
        printf("qwae:%s\n\n", buffer);
        Config_GetElement(config, "sad", buffer, sizeof(buffer));
        printf("sad:%s\n\n", buffer);
        Config_GetElement(config, "----", buffer, sizeof(buffer));
        printf("----:%s\n\n", buffer);

        LogDebug("setup", "SDL_SetRenderDrawColor");
        SDL_SetRenderDrawColor(app->Renderer, 160, 160, 160, 255);

        LogDebug("setup", "IMG_Load: loading <images/point.png> to <tmp_surf>");
        SDL_Surface *tmp_surf = IMG_Load("images/point.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        
        LogDebug("setup", "SDL_CreateTextureFromSurface: creating <point_texture>");
        point_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == point_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }
        
        LogDebug("setup", "IMG_Load: loading <images/ground.png> to <tmp_surf>");
        tmp_surf = IMG_Load("images/ground.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }

        LogDebug("setup", "SDL_CreateTextureFromSurface: creating <background_texture>");
        background_texture = SDL_CreateTextureFromSurface(app->Renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == background_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }

        LogDebug("setup", "Label_New: creating label <point_text>");
        point_text = Label_New(app->Renderer, "fonts/ArialBlackPrimer.ttf", "(0, 0)", TEXT_SIZE, TEXT_COLOR_Black, LABEL_PARAM_BORDER, TEXT_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }


        // Menu
        LogDebug("setup", "Label_New: creatng menu label <0>");
        menu_labels[0] = Label_New(app->Renderer, "fonts/Hasklig-Regular.ttf", "Add point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }
        
        LogDebug("setup", "Label_New: creatng menu label <1>");
        menu_labels[1] = Label_New(app->Renderer, "fonts/Hasklig-Regular.ttf", "Add to start", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        LogDebug("setup", "Label_New: creatng menu label <2>");
        menu_labels[2] = Label_New(app->Renderer, "fonts/Hasklig-Regular.ttf", "Delete point", 60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }


        LogDebug("setup", "Menu_New: creating menu: <menu>");
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

        LogDebug("setup", "destroying trash");
        SDL_DestroySurface(tmp_surf);

        LogDebug("setup", "end setup");

        return 1;
}





int Tick(APP *app) {
        SDL_Event event;
        static double mouse_x = 0;
        static double mouse_y = 0;

        while ( SDL_PollEvent(&event) ) {
                switch (event.type) {
                        case SDL_EVENT_QUIT:
                                app->is_running = 0;
                                return 0;
                                break;
                        case SDL_EVENT_MOUSE_MOTION:
                                mouse_x = event.motion.x;
                                mouse_y = event.motion.y;
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
                                        default:
                                                break;
                                }
                                break;
                        default:
                                break;
                }
        }

        static bool prev_lmb_state = 0;
        bool lmb_clicked = lmb_pressed && prev_lmb_state == 0;

        static bool prev_rmb_state = 0;
        bool rmb_clicked = rmb_pressed && prev_rmb_state == 0;

        
        changes |= CheckMousePos(&points, mouse_x, mouse_y, lmb_pressed, prev_lmb_state, shift_pressed, ctrl_pressed);
        
        static Point *to_change = NULL;
        if ( rmb_clicked && ( menu->active == 0 || Menu_MouseOut(menu, mouse_x, mouse_y) ) ) {
                Menu_Move(menu, mouse_x, mouse_y, APP_WIDTH, APP_HEIGHT);
                menu->active = 1;
                changes = 1;
                to_change = (Point*)( (uint64_t)points.selected_point | (uint64_t)points.selected_line );

                if ( to_change == NULL ) {
                        menu_buttons[2]->active = 0;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_Grey);
                } else {
                        menu_buttons[2]->active = 1;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_White);
                }
        }

        changes |= Menu_CheckUpdate(menu, mouse_x, mouse_y, lmb_clicked | rmb_clicked, to_change);

        prev_lmb_state = lmb_pressed;
        prev_rmb_state = rmb_pressed;

        return 1;
}



int main() {
        Logs_SetFile("logs.log");
        Logs_SetLogLevel(LOG_LEVEL_INFO);
        Logs_EnableColors(0);

        if ( 0==SDL_Init(SDL_INIT_FLAGS) ) {
                LogError("main", "SDL_Init failed: %s", SDL_GetError());
                return 0;
        }

        if ( 0==TTF_Init() ) {
                LogError("setup/TTF_Init", "TTF_Init failed: %s", SDL_GetError());
                return 0;
        }   

        app = AppNew("Планировщик маршрута", APP_WIDTH, APP_HEIGHT, 0, NULL);
        if ( NULL==app ) {
                LogError("main", "AppNew failed");
                goto app_quit;
        }
 
        if ( 0==setup(app) ) {
                LogError("main", "setup failed");
                goto app_quit;
        }

        AppSetTick(app, Tick);
        AppSetRendererTick(app, render);

        AppSetTps(app, TPS);
        AppSetFps(app, FPS);

        AppMainloop(app);

        app_quit:
        Label_Free(point_text);
        Menu_Free(menu);
        Config_Delete(config);

        SDL_DestroyTexture(point_texture);
        SDL_DestroyTexture(background_texture);
        AppQuit(app);
        TTF_Quit();
        SDL_Quit();
        return 0;
}
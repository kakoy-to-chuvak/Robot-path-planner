#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>

// Включаем режим колбэков SDL3
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "parametrs.h"
#include "label.h"
#include "logs.h"
#include "drawing.h"
#include "vectors.h"
#include "path.h"
#include "contextmenu.h"
#include "filesave.h"
#include "crossplatform.h"

// ---- Глобальные переменные ( вместо APP*) ----
static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;

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

// ---- Вспомогательные статические переменные для состояния ----
static SDL_FPoint mouse_pos = {0, 0};
static struct menu_args args = {NULL, {0, 0}};

// ---- Прототипы функций меню ( контекстное меню) ----
void *Menu_AddPoint(void *contextmenu, void *args_vpointer ) {
        struct menu_args args = *((struct menu_args*)args_vpointer);
        SDL_FPoint cords = WindowCordsToBox(args.cords, &parametrs);
        AddPoint(&points, cords, NULL, args.point, NULL, &parametrs);
        return contextmenu;
}

void *Menu_DelPoint(void *contextmenu, void *args_vpointer ) {
        struct menu_args args = *((struct menu_args*)args_vpointer);
        if ( args.point ) {
                DelPoint(&points, args.point);
        }
        return contextmenu;
}

void *Menu_AddPointToStart(void *contextmenu, void *args_vpointer ) {
        struct menu_args args = *((struct menu_args*)args_vpointer);
        SDL_FPoint cords = WindowCordsToBox(args.cords, &parametrs);
        AddPoint_tostart(&points, cords, 0, NULL, &parametrs);
        return contextmenu;
}

// ---- Функция рендеринга ----
int render(SDL_Renderer *renderer ) {
        LogTrace("render", "render");

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderTexture(renderer, background_texture, NULL, &parametrs.texture_box);

        RenderPath(renderer, point_texture, &points, point_text, &parametrs);

        ContextMenu_Render(contextmenu);

        SDL_RenderPresent(renderer);
        return 1;
}

// ---- Функция инициализации ----
int setup(SDL_Renderer *renderer, SDL_Window *window ) {
        LogDebug("setup", "entering setup");

        // Загрузка текстуры точки
        SDL_Surface *tmp_surf = IMG_Load("images/point.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }
        point_texture = SDL_CreateTextureFromSurface(renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == point_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }

        // Загрузка текстуры фона
        tmp_surf = IMG_Load("images/ground.png");
        if ( NULL == tmp_surf ) {
                LogError("setup", "IMG_Load failed: %s", SDL_GetError());
                return 0;
        }
        background_texture = SDL_CreateTextureFromSurface(renderer, tmp_surf);
        SDL_DestroySurface(tmp_surf);
        if ( NULL == background_texture ) {
                LogError("setup", "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                return 0;
        }

        // Создание метки для координат точки
        point_text = Label_New(renderer, "fonts/" POINT_CORDS_FONT, "(0, 0)",
                                                   TEXT_SIZE, TEXT_COLOR_Black, LABEL_PARAM_BORDER, TEXT_PARAMS);
        if ( NULL == point_text ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        // Создание меток для контекстного меню
        menu_labels[0] = Label_New(renderer, "fonts/" MENU_TEXT_FONT, "Add point",
                                                           60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == menu_labels[0] ) {
                LogError("setup", "Label_New failed");
                return 0;
        }
        menu_labels[1] = Label_New(renderer, "fonts/" MENU_TEXT_FONT, "Add point to start",
                                                           60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == menu_labels[1] ) {
                LogError("setup", "Label_New failed");
                return 0;
        }
        menu_labels[2] = Label_New(renderer, "fonts/" MENU_TEXT_FONT, "Delete point",
                                                           60, TEXT_COLOR_White, 0, LABEL_VOID_PARAMS);
        if ( NULL == menu_labels[2] ) {
                LogError("setup", "Label_New failed");
                return 0;
        }

        // Создание контекстного меню
        contextmenu = ContextMenu_New(renderer, SDL_PIXELFORMAT_RGBA32,
                                                                  MENU_BG, 5, 0, MENU_BORDER_COLOR);
        if ( NULL == contextmenu ) {
                LogError("setup", "Menu_New failed");
                return 0;
        }

        // Настройка кнопок меню
        ContextMenu_SetupButtons(contextmenu, 6, 170, 30,
                                                         MENU_BG, MENU_TRIGGER_COLOR, 4, 4, 5, 6);
        menu_buttons[2] = ContextMenu_SetButton(contextmenu, 2, menu_labels[2], 0, 1, Menu_DelPoint);
        menu_buttons[1] = ContextMenu_SetButton(contextmenu, 1, menu_labels[1], 0, 1, Menu_AddPointToStart);
        menu_buttons[0] = ContextMenu_SetButton(contextmenu, 0, menu_labels[0], 0, 1, Menu_AddPoint);

        // Настройки рендеринга
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(point_texture, 0, 0, 0);

        // Первый рендер
        render(renderer);

        LogDebug("setup", "Exit setup");
        return 1;
}

// ---- SDL_AppInit: инициализация приложения ----
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[] ) {
        // --- Работа с путями ( как в main) ---
        char user_path[MAX_PATH] = "";
        if ( getcwd(user_path, sizeof(user_path)) == NULL ) {
                return SDL_APP_FAILURE;
        }

        char *last_slash = strrchr(argv[0], '\\');
        if ( last_slash == NULL ) {
                last_slash = strrchr(argv[0], '/');
                if ( last_slash == NULL ) {
                        return SDL_APP_FAILURE;
                }
        }
        *last_slash = '\0';
        CRP_chdir(argv[0]);

        // Настройка логов
        Logs_SetFile("logs.log");
        Logs_SetLogLevel(LOG_LEVEL_NOTICE);
        Logs_EnableColors(0);

        // Инициализация SDL
        if ( 0 == SDL_Init(SDL_INIT_FLAGS) ) {
                LogError("SDL_AppInit", "SDL_Init failed: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        // Инициализация SDL_ttf
        if ( 0 == TTF_Init() ) {
                LogError("SDL_AppInit", "TTF_Init failed: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        // Создание окна и рендерера
        if ( !SDL_CreateWindowAndRenderer("Планировщик маршрута", 600, 400,
                                                                         SDL_WINDOW_RESIZABLE, &g_window, &g_renderer) ) {
                LogError("SDL_AppInit", "SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        // Инициализация параметров
        ParametrsInit(&parametrs, g_window);

        // Вызов setup
        if ( 0 == setup(g_renderer, g_window) ) {
                LogError("SDL_AppInit", "setup failed");
                return SDL_APP_FAILURE;
        }

        // Возврат в исходный каталог ( как было в main)
        CRP_chdir(user_path);

        // Загрузка файла из аргумента командной строки ( если передан)
        if ( argc == 2 ) {
                strcpy_s(points.file_name, MAX_PATH, argv[1]);
                LoadPoints(&points, &parametrs);
        }

        // Отмечаем, что нужно перерисовать
        points.changed = 1;

        return SDL_APP_CONTINUE;
}

// ---- SDL_AppEvent: обработка событий ----
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event ) {
        switch ( event->type ) {
                case SDL_EVENT_QUIT:
                        return SDL_APP_SUCCESS;

                case SDL_EVENT_MOUSE_MOTION:
                        mouse_pos.x = event->motion.x;
                        mouse_pos.y = event->motion.y;
                        break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                        if ( event->button.button == SDL_BUTTON_LEFT ) {
                                parametrs.lmb_pressed = 1;
                        } else if ( event->button.button == SDL_BUTTON_RIGHT ) {
                                parametrs.rmb_pressed = 1;
                        }
                        break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                        if ( event->button.button == SDL_BUTTON_LEFT ) {
                                parametrs.lmb_pressed = 0;
                        } else if ( event->button.button == SDL_BUTTON_RIGHT ) {
                                parametrs.rmb_pressed = 0;
                        }
                        break;

                case SDL_EVENT_KEY_DOWN:
                        LogTrace("SDL_AppEvent", "Key down. Scancode: %i  Char: '%c'",
                                         event->key.scancode, event->key.key);
                        switch ( event->key.scancode ) {
                                case SDL_SCANCODE_ESCAPE:
                                        return SDL_APP_SUCCESS;  // выход

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
                                        if ( parametrs.ctrl_pressed ) {
                                                LogDebug("SDL_AppEvent", "Saving file");
                                                if ( parametrs.shift_pressed || *points.file_name == '\0' ) {
                                                        file_save_args = ( FileSaveArgs){&points, &parametrs};
                                                        ShowSaveFIleDialog(NULL, points.file_name, &file_save_args);
                                                } else {
                                                        SavePoints(&points);
                                                }
                                        }
                                        break;

                                case SDL_SCANCODE_O:
                                        if ( parametrs.ctrl_pressed ) {
                                                file_save_args = ( FileSaveArgs){&points, &parametrs};
                                                LogDebug("SDL_AppEvent", "Opening file");
                                                ShowOpenFIleDialog(NULL, points.file_name, &file_save_args);
                                        }
                                        break;

                                case SDL_SCANCODE_F11:
                                        if ( SDL_GetWindowFlags(g_window) & SDL_WINDOW_MAXIMIZED ) {
                                                SDL_RestoreWindow(g_window);
                                        } else {
                                                SDL_MaximizeWindow(g_window);
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
                        switch ( event->key.scancode ) {
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
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                        ParametrsFixValues(&parametrs, g_window);
                        points.changed = 1;
                        break;

                default:
                        break;
        }
        return SDL_APP_CONTINUE;
}

// ---- SDL_AppIterate: основной игровой цикл ( вызывается каждый кадр) ----
SDL_AppResult SDL_AppIterate(void *appstate ) {
        // Обновление состояния мыши ( выделение точки под курсором)
        CheckMousePos(&points, mouse_pos, &parametrs);

        // Вычисление кликов ( кнопка только что отпущена)
        bool lmb_clicked = ( parametrs.lmb_pressed == 0 && parametrs.prev_lmb_state);
        bool rmb_clicked = ( parametrs.rmb_pressed == 0 && parametrs.prev_rmb_state);

        // Активация контекстного меню по правому клику
        if ( rmb_clicked && ( contextmenu->active == 0 ||
                                                ContextMenu_MouseOut(contextmenu, mouse_pos.x, mouse_pos.y)) ) {
                args.cords = mouse_pos;
                ContextMenu_Move(contextmenu, mouse_pos.x, mouse_pos.y,
                                                 parametrs.window_w, parametrs.window_h);
                contextmenu->active = 1;
                points.changed = 1;

                args.point = points.selected_point;

                // Обновление состояния кнопки "Delete point"
                if ( args.point == NULL ) {
                        menu_buttons[2]->active = 0;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_Grey);
                } else {
                        menu_buttons[2]->active = 1;
                        Label_Update(menu_labels[2], "Delete point", TEXT_COLOR_White);
                }
        }

        // Обновление контекстного меню ( обработка нажатий на кнопки)
        points.changed |= ContextMenu_CheckUpdate(contextmenu, mouse_pos.x, mouse_pos.y,
                                                                                          lmb_clicked | rmb_clicked, &args);

        // Рендеринг, если были изменения
        if ( points.changed ) {
                render(g_renderer);
                points.changed = 0;
        }

        // Сохраняем предыдущее состояние кнопок мыши
        parametrs.prev_lmb_state = parametrs.lmb_pressed;
        parametrs.prev_rmb_state = parametrs.rmb_pressed;

        return SDL_APP_CONTINUE;
}

// ---- SDL_AppQuit: освобождение ресурсов ----
void SDL_AppQuit(void *appstate, SDL_AppResult result ) {
        FreePoints(&points);
        PathClearActions();
        Label_Free(point_text);

        ContextMenu_Free(contextmenu);
        Label_Free(menu_labels[0]);
        Label_Free(menu_labels[1]);
        Label_Free(menu_labels[2]);

        SDL_DestroyTexture(point_texture);
        SDL_DestroyTexture(background_texture);

        if ( g_renderer) SDL_DestroyRenderer(g_renderer);
        if ( g_window)   SDL_DestroyWindow(g_window);

        TTF_Quit();
        SDL_Quit();
}
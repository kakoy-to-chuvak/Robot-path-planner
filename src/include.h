#ifndef INCLUDE_H
#define INCLUDE_H


#define SDL_INIT_FLAGS (SDL_INIT_VIDEO)
#define APP_TITLE "Планировщик маршрута"
#define APP_WIDTH 810 
#define APP_HEIGHT 540

#define TPS 60

#define TEXT_SIZE (35)

#define TEXT_COLOR_Black ((SDL_Color){0, 0, 0, 255})
#define TEXT_COLOR_BrightWhite ((SDL_Color){255, 255, 255, 255})
#define TEXT_COLOR_White ((SDL_Color){222, 222, 222, 255})
#define TEXT_COLOR_Grey ((SDL_Color){128, 128, 128, 255})

#define TEXT_PARAMS ((LABEL_PARAMS){TEXT_COLOR_White, 3})


#define MENU_BG ((SDL_Color){31, 31, 31, 255})
#define MENU_BORDER_COLOR ((SDL_Color){255, 0, 0, 120})
#define MENU_TRIGGER_COLOR ((SDL_Color){0, 123, 208, 255})


#define POINT_DIAMETR (POINT_RADIUS + POINT_RADIUS)
#define POINT_RADIUS 17

#define LINE_RADIUS 6
#define ARROW_BASE 20

#define BOX_HEIGHT 2.0f
#define BOX_WIDTH  3.0f



// 90°  (ALT248)



#endif  // INCLUDE_H
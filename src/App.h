#ifndef MYAPP_H
#define MYAPP_H


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "crossplatform.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_time.h>

#include "logs.h"


// ----- App structs -----
typedef struct APPPROPS 
{   
        SDL_Surface *Icon;
        char *Title;
        
        int width;
        int height;

        int preset_tps;
        int tps;
} APPPROPS;

typedef struct APP 
{   
        SDL_Window *Window;
        SDL_Renderer *Renderer;

        APPPROPS Properties;

        int (*Tick)(struct APP*);
        
        int exit_code;
        int is_running;
} APP;




APP *AppNew(const char *title, int width, int height, SDL_WindowFlags window_flags, const char *icon);
void AppSetTick(APP *app, int(*Tick)(APP*));
void AppSetRendererTick(APP *app, int(*_Tick)(APP*));
int AppMainloop(APP *app);


void AppSetTps(APP *app, int tps);
int AppGetTps(APP *app);


void AppQuit(APP *app);






#endif // MYAPP_H
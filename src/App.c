 #include "App.h"



// ----- App functions -----
APP *AppNew(const char *title, int width, int height, SDL_WindowFlags window_flags, const char *icon) 
{
        APP *app = (APP*)malloc(sizeof(APP)); 
        if ( app==NULL ) {
                LogError("AppNew/malloc", "malloc failed");
                goto failure0;
        }

        app->Properties.Icon = NULL;
        app->Properties.Title = NULL;

        if ( NULL==title )
                goto no_title;

        app->Properties.Title = (char*)malloc(strlen(title)+1);
        if ( NULL==app->Properties.Title ) {
                LogError("AppNew/malloc", "malloc failed");
                goto failure1; 
        }
        strcpy(app->Properties.Title, title);

        no_title:
                        

        app->Window = SDL_CreateWindow(title, width, height, window_flags);
        if ( NULL==app->Window ) {
                LogError("AppNew/SDL_CreateWindow", "SDL_CreateWindow failed: %s", SDL_GetError());  
                goto failure2;
        }

        if ( NULL==icon )
                goto no_icon;

        app->Properties.Icon = IMG_Load(icon);
        if ( app->Properties.Icon == NULL ) {
                LogError("AppNew/IMG_Load", "Failed to load \"%s\": %s", icon, SDL_GetError());
                goto failure3;
        }

        if ( 0==SDL_SetWindowIcon(app->Window, app->Properties.Icon) ) {
                LogWarn("AppNew/SDL_SetWindowIcon", "Failed to set icon: %s", SDL_GetError());
        }

        no_icon:

        app->Renderer = SDL_CreateRenderer(app->Window, NULL);
        if ( NULL==app->Renderer ) {
                LogError("AppNew/SDL_CreateRenderer", "SDL_CreateRenderer failed: %s", SDL_GetError());
                goto failure4;
        }

        app->Properties.height = height;
        app->Properties.width = width;
        app->Properties.preset_tps = 180;
        app->Properties.preset_tps = 60;
        app->is_running = 1;
        app->exit_code = 0;

        return app;

        failure4:
        if ( icon ) 
                SDL_DestroySurface(app->Properties.Icon);
        failure3:
        SDL_DestroyWindow(app->Window);
        failure2:
        if ( title ) 
                free(app->Properties.Title);
        failure1:
        free(app);
        failure0:
        return NULL;
}



int AppMainloop(APP *app) {
        LogDebug("AppMainloop", "set variables");;
        uint64_t last_tick = 0;
        uint64_t ns_per_tick = 1000000000 / app->Properties.preset_tps;

        LogDebug("AppMainloop", "set tick functions");
        int (*Tick)(APP*) = app->Tick;

        LogDebug("AppMainloop", "start cycle");
        while ( app->is_running && app->exit_code >= 0) {
                
                if ( SDL_GetTicksNS() - last_tick > ns_per_tick ) {
                        app->Properties.tps = (int)round((1000000000.f/(double)(SDL_GetTicksNS()-last_tick)));
                        last_tick = SDL_GetTicksNS();
                        if ( Tick )
                                app->exit_code = Tick(app);
                }

                CRP_sleep(1);
        }

        LogDebug("AppMainloop", "return");
        return app->exit_code;
}

void AppSetTick(APP *app, int(*Tick)(APP*)) { 
        app->Tick = Tick; 
}


// fps / tps 
void AppSetTps(APP *app, int tps) {
        app->Properties.preset_tps = tps;
}


int AppGetTps(APP *app) {         
        return app->Properties.tps; 
}



// Quit
void AppQuit(APP *app) 
{   
        if ( NULL==app ) 
                return;

        if ( app->Window )
                SDL_DestroyWindow(app->Window);

        if ( app->Renderer )
                SDL_DestroyRenderer(app->Renderer);

        if ( app->Properties.Title )
                free(app->Properties.Title);
        
        if ( app->Properties.Icon )
                SDL_DestroySurface(app->Properties.Icon);
        
        free(app);
}
#include "filesave.h"

// dialog filters
static const SDL_DialogFileFilter dialog_filters[4] = {
    { "JSON (*.json)",    "json" },
    { "All files (*.*)",  "*"    }
};



void SavePoints(PArray* _Points) {
        // check null pointers
        if ( _Points == NULL ) {
                LogNotice("SavePoints", "No points to save");
                return;
        }

        // open file
        FILE *file = fopen(_Points->file_name, "w");
        if ( file == NULL ) {
                LogNotice("SavePoints", "Error on opening file: %s", _Points->file_name);
                return;
        }

        fputc('[', file);
        Point *now = _Points->points;
        while ( now ) {
                fprintf(file, "{\"x\":%.4f,\"y\":%.4f,\"angle\":%.10f},", now->cords.x, now->cords.y, now->angle);
                now = now->next;
        }

        // close json brecket
        fseek(file, -1, SEEK_CUR);
        if ( ftell(file) != 0 ) {
                fputc(']', file);
        }
        
        // close file
        fclose(file);
}


static void SDLCALL __SaveFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
        // check null pointers
        if ( filelist == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "An error occured: %s", SDL_GetError());
                return;
        }

        if ( *filelist == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "No files selected");
                return;
        }

        // get args and check null pointers
        FileSaveArgs *args = (FileSaveArgs*)userdata;
        if ( args == NULL || args->points == NULL || args->parametrs == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "No args received");
                return;
        }

        if ( filter == 0 ) {
                char *extension = strrchr(*filelist, '.');

                // if extension is undefined or not equals ".json"
                if ( extension == NULL || strcmp(extension, ".json") ) {
                        strcat_s(args->points->file_name, MAX_PATH, ".json");
                }
        }
        
        // add changes to PArray
        strcpy_s(args->points->file_name, MAX_PATH, *filelist);

        // save PArray
        SavePoints(args->points);
}

// decorator function
void ShowSaveFIleDialog(SDL_Window *_Window, const char *_Default_location, FileSaveArgs *_Args) {
        SDL_ShowSaveFileDialog(__SaveFileDialogCallback, _Args, _Window, dialog_filters, 2, _Default_location);
}







void LoadPoints(PArray* _Points, Parametrs *_Parametrs) {
        FreePoints(_Points);

        _Points->points = NULL;

        FILE *file = fopen(_Points->file_name, "r");

        __ParseJSON(_Points, file, _Parametrs);
        
        fclose(file);

        _Points->changed = 1;
}


static void SDLCALL __OpenFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
        if ( filelist == NULL ) {
                LogNotice("ShowOpenFIleDialog (__OpenFileDialogCallback)", "An error occured: %s", SDL_GetError());
                return;
        }

        if ( *filelist == NULL || filter  == -1 ) {
                LogNotice("ShowOpenFIleDialog (__OpenFileDialogCallback)", "No files selected");
                return;
        }

        FileSaveArgs *args = (FileSaveArgs*)userdata;
        if ( args == NULL || args->points == NULL || args->parametrs == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "No args received");
                return;
        }

        strcpy_s(args->points->file_name, MAX_PATH, *filelist);

        LoadPoints(args->points, args->parametrs);
}


void ShowOpenFIleDialog(SDL_Window *_Window, const char *_Default_location, FileSaveArgs *_Args) {
        SDL_ShowOpenFileDialog(__OpenFileDialogCallback, _Args, _Window, dialog_filters, 2, _Default_location, 0);
}
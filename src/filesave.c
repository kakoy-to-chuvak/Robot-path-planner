#include "filesave.h"


static const SDL_DialogFileFilter dialog_filters[4] = {
    { "JSON files",   "json" },
    { "TXT files",   "txt" },
    { "CSV files",   "csv" },
    { "All files",   "*" }
};


void SavePoints(PArray* _Points, const char *_Filename, FILESAVE_FORMAT _Save_format) {
        if ( _Points == NULL ) {
                LogNotice("SavePoints", "No points to save");
                return;
        }

        FILE *file = fopen(_Filename, "w");
        if ( file == NULL ) {
                LogNotice("SavePoints", "Error on opening file: %s", _Filename);
                return;
        }

        Point *now = _Points->points;

        if ( _Save_format == SAVE_FORMAT_UNDEFINED ) {
                // define save format by extension
                char *extension = strrchr(_Filename, '.');
                if ( extension == NULL ) {
                        _Save_format = SAVE_FORMAT_TXT;
                        goto save_points;
                }

                extension++;
                
                if ( strcmp(extension, "csv") == 0 ) {
                        _Save_format = SAVE_FORMAT_CSV;
                        goto save_points;
                } else if ( strcmp(extension, "json") == 0 ) {
                        _Save_format = SAVE_FORMAT_JSON;
                        goto save_points;
                } else {
                        _Save_format = SAVE_FORMAT_TXT;
                        goto save_points;
                }
        }

        save_points:
        char *print_format = "%f %f\n";

        switch (_Save_format) {
                case SAVE_FORMAT_JSON:
                        print_format = "[%f,%f],";
                        fputc('[', file);
                        break;
                case SAVE_FORMAT_CSV:
                        print_format = "%f,%f\n";
                        fputs("x,y\n", file);
                        break;
                default:
                        break;
        }

        while ( now ) {
                fprintf(file, print_format, now->cords.x, now->cords.y);
                now = now->next;
        }

        if ( _Save_format == SAVE_FORMAT_JSON ) {
                fseek(file, -1, SEEK_CUR);
                fputc(']', file);
        }

        fclose(file);
}


static void SDLCALL FileDialogCallback(void* userdata, const char* const* filelist, int filter) {
        if ( filelist == NULL ) {
                LogNotice("OpenFIleDialog (FileDialogCallback)", "An error occured: %s", SDL_GetError());
                return;
        }

        if ( *filelist == NULL ) {
                LogNotice("OpenFIleDialog (FileDialogCallback)", "No files selected");
                return;
        }

        FILESAVE_FORMAT format = SAVE_FORMAT_UNDEFINED;
        if ( filter > 0 && filter <= 4 ) {
                format = (FILESAVE_FORMAT)filter;
        }

        while (*filelist) {
                SavePoints(userdata, *filelist, format);
                filelist++;
        }
}





void OpenFIleDialog(SDL_Window *_Window, const char *_Default_location, PArray *_Points) {
        SDL_ShowOpenFileDialog(FileDialogCallback, _Points, _Window, dialog_filters, 4, _Default_location, 1);
}
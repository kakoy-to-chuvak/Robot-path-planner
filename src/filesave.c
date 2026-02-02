#include "filesave.h"


static const SDL_DialogFileFilter dialog_filters[4] = {
    { "JSON (*.json)",   "json" },
    { "Points (*.pts)",  "pts" },
    { "CSV (*.csv)",     "csv" },
    { "All files (*.*)", "*" }
};


void SavePoints(PArray* _Points) {
        if ( _Points == NULL ) {
                LogNotice("SavePoints", "No points to save");
                return;
        }

        FILE *file = fopen(_Points->file_name, "w");
        if ( file == NULL ) {
                LogNotice("SavePoints", "Error on opening file: %s", _Points->file_name);
                return;
        }

        char *print_format = "%.3f %.3f\n";

        switch (_Points->format) {
                case FILE_FORMAT_JSON:
                        print_format = "[%.3f,%.3f],";
                        fputc('[', file);
                        if ( _Points->points == NULL ) {
                                fputc(' ', file);
                        }
                        break;
                case FILE_FORMAT_CSV:
                        print_format = "%.3f,%.3f\n";
                        fputs("x,y\n", file);
                        break;
                default:
                        break;
        }

        Point *now = _Points->points;
        while ( now ) {
                fprintf(file, print_format, now->cords.x, now->cords.y);
                now = now->next;
        }

        if ( _Points->format == FILE_FORMAT_JSON ) {
                fseek(file, -1, SEEK_CUR);
                fputc(']', file);
        }

        fclose(file);
}


static void SDLCALL __SaveFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
        if ( filelist == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "An error occured: %s", SDL_GetError());
                return;
        }

        if ( *filelist == NULL ) {
                LogNotice("ShowSaveFIleDialog (__SaveFileDialogCallback)", "No files selected");
                return;
        }

        FILESAVE_FORMAT format = FILE_FORMAT_UNDEFINED;
        if ( filter >= 0 && filter < 3 ) {
                format = (FILESAVE_FORMAT)filter;
        } else {
                // define save format by extension
                char *extension = strrchr(*filelist, '.');
                if ( extension == NULL ) {
                        format = FILE_FORMAT_PTS;
                        goto save_points;
                }

                extension++;
                
                if ( strcmp(extension, "csv") == 0 ) {
                        format = FILE_FORMAT_CSV;
                } else if ( strcmp(extension, "json") == 0 ) {
                        format = FILE_FORMAT_JSON;
                } else {
                        format = FILE_FORMAT_PTS;
                }
        }

        save_points:

        ((PArray*)userdata)->format = format;
        strcpy_s(((PArray*)userdata)->file_name, MAX_PATH, *filelist);

        SavePoints(userdata);
}





void ShowSaveFIleDialog(SDL_Window *_Window, const char *_Default_location, PArray *_Points) {
        SDL_ShowSaveFileDialog(__SaveFileDialogCallback, _Points, _Window, dialog_filters, 4, _Default_location);
}


// void __ParseCSV(PArray *_Points, FILE *stream) {
        
// }

void __ParsePTS(PArray *_Points, FILE *stream) {
        float x = 0;
        float y = 0;

        char buffer[1024] = "";

        while ( fgets(buffer, sizeof(buffer), stream) ) {
                int n = sscanf(buffer, "%f %f", &x, &y);
                if ( n == 2 ) {
                        AddPoint(_Points, (SDL_FPoint){x,y}, NULL);
                } else {
                        n = strlen(buffer) - 1;
                        if ( buffer[n] == '\n' ) 
                                buffer[n] = '\0';
                        LogNotice("ShowOpenFIleDialog (LoadPoints)", "wrong format for \".pts\": \"%s\"", buffer);
                }
        }
}

// void __ParseJSON(PArray *_Points, FILE *stream) {
        
// }


void LoadPoints(PArray* _Points) {
        FreePoints(_Points);
        _Points->points = NULL;

        FILE *file = fopen(_Points->file_name, "r");
        
        switch ( _Points->format ) {
                case FILE_FORMAT_CSV:
                        __ParsePTS(_Points, file);
                        break;
                case FILE_FORMAT_JSON:
                        __ParsePTS(_Points, file);
                        break;
                default:
                        __ParsePTS(_Points, file);
        }

        _Points->changed = 1;
}


static void SDLCALL __OpenFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
        if ( filelist == NULL ) {
                LogNotice("ShowOpenFIleDialog (__OpenFileDialogCallback)", "An error occured: %s", SDL_GetError());
                return;
        }

        if ( *filelist == NULL ) {
                LogNotice("ShowOpenFIleDialog (__OpenFileDialogCallback)", "No files selected");
                return;
        }

        FILESAVE_FORMAT format = FILE_FORMAT_UNDEFINED;
        if ( filter >= 0 && filter < 3 ) {
                format = (FILESAVE_FORMAT)filter;
        } else {
                // define save format by extension
                char *extension = strrchr(*filelist, '.');
                if ( extension == NULL ) {
                        format = FILE_FORMAT_PTS;
                        goto save_points;
                }

                extension++;
                
                if ( strcmp(extension, "csv") == 0 ) {
                        format = FILE_FORMAT_CSV;
                } else if ( strcmp(extension, "json") == 0 ) {
                        format = FILE_FORMAT_JSON;
                } else {
                        format = FILE_FORMAT_PTS;
                }
        }

        save_points:

        ((PArray*)userdata)->format = format;
        strcpy_s(((PArray*)userdata)->file_name, MAX_PATH, *filelist);

        LoadPoints(userdata);
}


void ShowOpenFIleDialog(SDL_Window *_Window, const char *_Default_location, PArray *_Points) {
        SDL_ShowOpenFileDialog(__OpenFileDialogCallback, _Points, _Window, dialog_filters, 4, _Default_location, 0);
}
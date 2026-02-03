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

        char *print_format = "%.4f %.4f %.10f\n";

        switch (_Points->format) {
                case FILE_FORMAT_JSON:
                        print_format = "{\"x\":%.4f,\"y\":%.4f,\"angle\":%.10f],";
                        fputc('[', file);
                        if ( _Points->points == NULL ) {
                                fputc(' ', file);
                        }
                        break;
                case FILE_FORMAT_CSV:
                        print_format = "%.4f,%.4f,%.10f\n";
                        fputs("x,y,angle\n", file);
                        break;
                default:
                        break;
        }

        Point *now = _Points->points;
        while ( now ) {
                fprintf(file, print_format, now->cords.x, now->cords.y, now->angle);
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

        char *extension = strrchr(*filelist, '.');
        if ( extension ) {
                extension++;
        }

        FILESAVE_FORMAT format = FILE_FORMAT_UNDEFINED;
        if ( filter >= 0 && filter < 3 ) {
                format = (FILESAVE_FORMAT)filter;
        } else {
                // define save format by extension
                if ( extension == NULL ) {
                        format = FILE_FORMAT_PTS;
                        goto save_points;
                }
                
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

        switch ( filter ) {
                case 0: 
                        if ( extension == NULL || strcmp(extension, "json") )
                                strcat_s(((PArray*)userdata)->file_name, MAX_PATH, ".json");
                        break;
                case 1:
                        if ( extension == NULL || strcmp(extension, "pts") )
                                strcat_s(((PArray*)userdata)->file_name, MAX_PATH, ".pts");
                        break;
                case 2:
                        if ( extension == NULL || strcmp(extension, "csv") )
                                strcat_s(((PArray*)userdata)->file_name, MAX_PATH, ".csv");
                        break;
                default:
                        break;
        } 

        SavePoints(userdata);
}


void ShowSaveFIleDialog(SDL_Window *_Window, const char *_Default_location, PArray *_Points) {
        SDL_ShowSaveFileDialog(__SaveFileDialogCallback, _Points, _Window, dialog_filters, 4, _Default_location);
}





void __ParseCSV(PArray *_Points, FILE *_Stream) {
        float x = 0;
        float y = 0;
        float angle = 0;

        char buffer[1024] = "";

        if ( fgets(buffer, sizeof(buffer), _Stream) == 0 ) {
                LogNotice("ShowOpenFIleDialog (LoadPoints)", "couldn`t read file");  
        }

        int n = strlen(buffer) - 1;
        if ( buffer[n] == '\n' ) 
                buffer[n] = '\0';

        if ( strcmp(buffer, "x,y,angle") ) {
                LogNotice("ShowOpenFIleDialog (LoadPoints)", "wrong format for <.csv>: \"%s\" expected \"x,y,angle\"", buffer);
                return;
        }

        while ( fgets(buffer, sizeof(buffer), _Stream) ) {
                int n = sscanf(buffer, "%f,%f,%f", &x, &y, &angle);
                if ( n == 3 ) {
                        AddPoint(_Points, (SDL_FPoint){x,y}, &angle, NULL);
                } else {
                        n = strlen(buffer) - 1;
                        if ( buffer[n] == '\n' ) 
                                buffer[n] = '\0';
                        LogNotice("ShowOpenFIleDialog (LoadPoints)", "wrong format for <.csv>: \"%s\"", buffer);
                }
        }
}

void __ParsePTS(PArray *_Points, FILE *_Stream) {
        float x = 0;
        float y = 0;
        float angle = 0;

        char buffer[1024] = "";

        while ( fgets(buffer, sizeof(buffer), _Stream) ) {
                int n = sscanf(buffer, "%f %f %f", &x, &y, &angle);
                if ( n == 3 ) {
                        AddPoint(_Points, (SDL_FPoint){x,y}, &angle, NULL);
                } else {
                        n = strlen(buffer) - 1;
                        if ( buffer[n] == '\n' ) 
                                buffer[n] = '\0';
                        LogNotice("ShowOpenFIleDialog (LoadPoints)", "wrong format for <.pts>: \"%s\"", buffer);
                }
        }
}

// void __ParseJSON(PArray *_Points, FILE *_Stream) {
//         float x = 0;
//         float y = 0;
//         float angle = 0;

//         // get file size
//         fseek(_Stream, 0 , SEEK_END);
//         long file_size = ftell(_Stream);
//         fseek(_Stream, 0, SEEK_SET);

//         if ( file_size > 134217728 ) {
//                 LogNotice("ShowOpenFIleDialog (LoadPoints)", "File too large (128MB)");
//                 return;
//         }

//         char *buffer = (char*)malloc(file_size + 1);

//         if ( fread(buffer, 1, file_size, _Stream) != file_size ) {
//                 LogNotice("ShowOpenFIleDialog (LoadPoints)", "couldn`t read file");
//                 free(buffer);
//                 return;
//         }




//         free(buffer);
// }


void LoadPoints(PArray* _Points) {
        FreePoints(_Points);
        _Points->points = NULL;

        FILE *file = fopen(_Points->file_name, "r");
        
        switch ( _Points->format ) {
                case FILE_FORMAT_CSV:
                        __ParseCSV(_Points, file);
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
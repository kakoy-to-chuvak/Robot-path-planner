#include "easy_json_parser.h"



#define NULL_TOKEN ((Token){NULL, TKNTP_UNDEFINED})
#define IS_SPACE(X)  ( X == ' ' || X == '\t' || X == '\n' )

enum TokenType {
        TKNTP_COMMA,
        TKNTP_COLON,
        TKNTP_OPEN_BRACKET,
        TKNTP_CLOSE_BRACKET,
        TKNTP_OPEN_F_BRACKET,
        TKNTP_CLOSE_F_BRACKET,
        TKNTP_STRING,
        TKNTP_NUMBER,
        TKNTP_TRUE,
        TKNTP_FALSE,
        TKNTP_UNDEFINED = -1,
};

typedef struct Token {
        char *token;
        enum TokenType type;
        struct Token *prev;
        struct Token *next;
} Token;


enum ObjectType {
        JSON_LIST,
        JSON_DICT,
        JSON_STRING,
        JSON_NUMBER,
        JSON_TRUE,
        JSON_FALSE,
};

typedef struct JsonObject {
        char *key;
        char *value;
        enum ObjectType type;
        struct JsonObject *childs;
        struct JsonObject *next;
        struct JsonObject *prev;
} JsonObject;



void FreeToken(Token *_Token) {
        if ( _Token == NULL ) {
                return;
        }

        free(_Token->token);
        free(_Token);
}

void FreeTokens(Token *_Tokens) {
        while ( _Tokens ) {
                FreeToken(_Tokens);
                _Tokens = _Tokens->next;
        }
}




Token *_GetToken(char **_Buffer) {
        if ( **_Buffer == '\0' ) {
                return NULL;
        }

        char *now = *_Buffer;
        size_t len = 1;
        
        bool string_opened = 0;
        char *lf_space = NULL; // last-first space

        // allocate memory for token object
        Token *new_token = calloc(1, sizeof(Token));
        if ( new_token == NULL ) {
                return NULL;
        }

        // trim token
        while ( IS_SPACE(*now) ) {
                if ( *now == '\0' ) {
                        return NULL;
                }
                now++;
        }

        *_Buffer = now;
        switch ( *now ) {
                case '\0':
                        return NULL;
                case ':':
                        new_token->type = TKNTP_COLON;
                        goto create_token;
                case '[':
                        new_token->type = TKNTP_OPEN_BRACKET;
                        goto create_token;
                case ']':
                        new_token->type = TKNTP_CLOSE_BRACKET;
                        goto create_token;
                case '{':
                        new_token->type = TKNTP_OPEN_F_BRACKET;
                        goto create_token;
                case '}':
                        new_token->type = TKNTP_CLOSE_F_BRACKET;
                        goto create_token;
                case ',':
                        new_token->type = TKNTP_COMMA;
                        goto create_token;
                case '\"':
                        new_token->type = TKNTP_STRING;
                        string_opened = 1;
                        break;
                default:
                        if ( *now >= '0' && *now <= '9' ) {
                                new_token->type = TKNTP_NUMBER;
                        } else if ( *now == '-' && now[1] >= '0' && now[1] <= '9' ) {
                                new_token->type = TKNTP_NUMBER;
                        } else {
                                new_token->type = TKNTP_UNDEFINED;
                        }
                        break;                
        }

        // parsing
        now++;
        bool prev_is_space = 0;
        while ( now ) {
                // check delimetrs
                if ( !string_opened && ( *now == ':' || *now == '[' || *now == ']' || *now == '{' || *now == '}' || *now == ',' ) ) {
                        break;
                }

                // check string ("")
                if ( *now == '\"' && now[-1] != '\\' ) {
                        string_opened = !string_opened;
                        // if string opened second time
                        if ( string_opened && new_token->type == TKNTP_STRING ) {
                                new_token->type = TKNTP_UNDEFINED;
                        }
                }

                // find last space for trim string
                if ( IS_SPACE(*now) ) {
                        if ( prev_is_space == 0 ) {
                                lf_space = now;
                                prev_is_space = 1;
                        }
                } else {
                        prev_is_space = 0;
                        lf_space = NULL;
                }
                
                // next char
                now++;
                len++;
        }

        // trim string
        if ( lf_space ) {
                *lf_space = '\0';
        }


        create_token:
        if ( string_opened ) {
                new_token->type = TKNTP_UNDEFINED;
        }

        if ( new_token->type == TKNTP_UNDEFINED ) {
                if ( len == 4 && strcmp(now, "True") == 0 ) {
                        new_token->type = TKNTP_TRUE;
                } else if ( len == 5 && strcmp(now, "False") == 0 ) {
                        new_token->type = TKNTP_FALSE;
                }
        }

        // allocate memory for token string
        new_token->token = calloc(len + 1, sizeof(char));
        if ( new_token->token == NULL ) {
                return NULL;
        }

        // copy token string
        now = *_Buffer;
        for ( size_t i = 0 ; i < len ; i++ ) {
                new_token->token[i] = now[i];
        }

        // trim token
        if ( lf_space ) {
                new_token->token[lf_space-now] = '\0';
        }

        *_Buffer += len;
        return new_token;
}


void _FreeObject(JsonObject *_Object) {
        while ( _Object != NULL ) {
                _FreeObject(_Object->childs);
                JsonObject *temp = _Object->next;
                free(_Object);
                _Object = temp;
        }
}

JsonObject *_GetObject(Token *_Tokens, Token **_Object_end) {
        if ( _Tokens == NULL ) {
                return NULL;
        }

        JsonObject *object = calloc(1, sizeof(JsonObject));
        JsonObject *now_object = object;

        get_object:
        switch ( _Tokens->type ) {
                case TKNTP_OPEN_BRACKET:
                        now_object->type = JSON_LIST;
                        now_object->childs = _GetObject(_Tokens->next, _Object_end);
                        _Tokens = *_Object_end;
                        if ( _Tokens == NULL || _Tokens->type != TKNTP_CLOSE_BRACKET ) {
                                LogNotice("ShowOpenFIleDialog (LoadPoints)", "Couldn`t create JSON object: unclosed bracket");
                                goto syntax_error;
                        }
                        break;
                
                case TKNTP_OPEN_F_BRACKET:
                        now_object->type = JSON_DICT;
                        now_object->childs = _GetObject(_Tokens->next, _Object_end);
                        _Tokens = *_Object_end;
                        if ( _Tokens == NULL || _Tokens->type != TKNTP_CLOSE_F_BRACKET ) {
                                LogNotice("ShowOpenFIleDialog (LoadPoints)", "Couldn`t create JSON object: unclosed f bracket");
                                goto syntax_error;
                        }
                        break;

                case TKNTP_STRING:
                        if ( _Tokens->next && _Tokens->next->type == TKNTP_COLON ) {
                                if ( _Tokens->next->next == NULL || now_object->key != NULL) {
                                        LogNotice("ShowOpenFIleDialog (LoadPoints)", "Couldn`t create JSON object: wrong sequence after colon");
                                        goto syntax_error;
                                }
                                now_object->key = _Tokens->token;
                                _Tokens = _Tokens->next->next;
                                goto get_object;
                        } else {
                                now_object->type = JSON_STRING;
                                now_object->value = _Tokens->token;
                        }       
                        break;

                case TKNTP_FALSE:
                        now_object->type = JSON_FALSE;
                        now_object->value = _Tokens->token;
                        break;
                
                case TKNTP_TRUE:
                        now_object->type = JSON_TRUE;
                        now_object->value = _Tokens->token;
                        break;

                case TKNTP_NUMBER:
                        now_object->type = JSON_TRUE;
                        now_object->value = _Tokens->token;
                        break;
                
                default:
                        LogNotice("ShowOpenFIleDialog (LoadPoints)", "Couldn`t create JSON object: undefined token type");
                        goto syntax_error;
                        
        }

        _Tokens = _Tokens->next;
        if ( _Tokens && _Tokens->type == TKNTP_COMMA ) {
                now_object->next = calloc(1, sizeof(JsonObject));
                now_object->next->prev = now_object;
                now_object = now_object->next;
                _Tokens = _Tokens->next;
                goto get_object;
        }
        
        *_Object_end = _Tokens;
        return object;


        syntax_error:
        _FreeObject(object);
        *_Object_end = NULL;
        return NULL;
}


void _PrintObject(JsonObject *_Object, int _Deep) {
        while ( _Object ) {
                printf("%*s", _Deep*4, "");
                if ( _Object->key ) {
                        printf("%s: ", _Object->key);
                }
                switch ( _Object->type ) {
                        case JSON_DICT:
                                printf("{\n");
                                _PrintObject(_Object->childs, _Deep+1);
                                printf("%*s}", _Deep*4, "");
                                break;

                        case JSON_LIST:
                                printf("[\n");
                                _PrintObject(_Object->childs, _Deep+1);
                                printf("%*s]", _Deep*4, "");
                                break;
                        
                        default:
                                printf("%s", _Object->value);
                                break;
                }

                _Object = _Object->next;
                if ( _Object ) {
                        putc(',', stdout);
                }
                putc('\n', stdout);
        }
}




void __ParseJSON(PArray *_Points, FILE *_Stream, Parametrs *_Parametrs) {
        // get file size
        fseek(_Stream, 0 , SEEK_END);
        size_t file_size = ftell(_Stream);
        fseek(_Stream, 0, SEEK_SET);

        if ( file_size > 134217728 ) {
                LogNotice("ShowOpenFIleDialog (LoadPoints)", "File too large (128MB)");
                return;
        }

        // allocate buffer
        char *buffer = (char*)malloc(file_size + 1);

        // read file
        size_t buffer_size = fread(buffer, 1, file_size, _Stream);
        if (  buffer_size == 0 ) {
                LogNotice("ShowOpenFIleDialog (LoadPoints)", "couldn`t read file");
                free(buffer);
                return;
        }

        buffer[buffer_size] = '\0';

        char *token_bufffer = buffer;

        // create array of tokens
        LogDebug("ShowOpenFIleDialog (LoadPoints)", "Tokenizing JSON");
        Token *token_list = _GetToken(&token_bufffer);
        token_list->prev = NULL;
        Token *token = token_list;
        while ( token ) {
                // printf(" %2i | %s\n", token->type, token->token);
                token->next = _GetToken(&token_bufffer);
                if ( token->next == NULL ) {
                        break;
                }
                token->next->prev = token;
                token = token->next;
        }

        Token *object_end = NULL;
        JsonObject *object = _GetObject(token_list, &object_end);
        if ( object == NULL ) {
                LogNotice("ShowOpenFIleDialog (LoadPoints)", "couldn`t create JSON object");
        }

        LogNotice("ShowOpenFIleDialog (LoadPoints)", "printing object");
        _PrintObject(object, 0);

        // free allocated mem
        _FreeObject(object);
        FreePoints(_Points);
        AddPoint(_Points, (SDL_FPoint){0,0}, (float*)&file_size, NULL, _Parametrs);
        FreeTokens(token_list);
        free(buffer);
}



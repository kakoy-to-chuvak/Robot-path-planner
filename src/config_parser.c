#include "config_parser.h"
#include "logs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>



uint64_t __hash(char *str) {
        if ( str == NULL ) {
                return 2166136261;
        } 

        uint64_t hash = 2166136261;
        while ( *str ) {
                hash ^= *str;
                hash *= 16777619;
                str++;
        }
        return hash;
}



struct __CONFIG_PARSED_MOULE__ *__Config_SetModule(PARSED_CONFIG *__Config, char *__Module_name) {
        uint64_t hash = __hash(__Module_name);

        struct __CONFIG_PARSED_MOULE__ *now = __Config->modules;

        int i = 0;
        while ( i < __Config->modules_count ) {
                if ( now->hash == hash && strcmp(__Module_name, now->name) == 0 ) {
                        break;
                }
                now++;
                i++;
        }

        if ( i < __Config->modules_count ) {
                return now;
        }

        struct __CONFIG_PARSED_MOULE__ *realloc_result = realloc(__Config->modules, sizeof(struct __CONFIG_PARSED_MOULE__) * ( i + 1 ) );
        if ( realloc_result == NULL ) {
                LogError("Config_Parse", "error on reallocating memory for [realloc_result]");
                return NULL;
        }

        __Config->modules = realloc_result;
        __Config->modules_count++;
        now = &realloc_result[i];

        now->elements_count = 0;
        now->elements = malloc(0);
        now->hash = hash;
        now->name = malloc( strlen(__Module_name) + 1 );
        strcpy(now->name, __Module_name);

        return now;
}

struct __CONFIG_PARSED_ELEMENT__ *__Module_AddElement(struct __CONFIG_PARSED_MOULE__ *__Module, char *__Key, char *__Value) {
        struct __CONFIG_PARSED_ELEMENT__ *realloc_result = realloc(__Module->elements, sizeof(struct __CONFIG_PARSED_ELEMENT__) * ( __Module->elements_count + 1 ) );
        if ( realloc_result == NULL ) {
                LogError("Config_Parse", "error on reallocating memory for [realloc_result]");
                return NULL;
        }

        struct __CONFIG_PARSED_ELEMENT__ *new_element = &realloc_result[__Module->elements_count];
        __Module->elements = realloc_result;
        __Module->elements_count++;

        new_element->key = malloc( strlen(__Key) + 1 );
        if ( new_element->key == NULL ) {
                __Module->elements_count--;
                return NULL;
        }

        new_element->raw_value = malloc( strlen(__Value) + 1 );
        if ( new_element->raw_value == NULL ) {
                __Module->elements_count--;
                free(new_element->key);
                return NULL;
        }
        new_element->value.raw = new_element->raw_value;
        new_element->type = VALUE_TYPE_RAW;

        strcpy(new_element->raw_value, __Value);
        strcpy(new_element->key, __Key);
        new_element->hash = __hash(__Key);

        return new_element; 
}

int __Compare_elements(const void *_Element_1, const void *_Element_2) {
        // compares elements hashes
        uint64_t hash1 = ((struct __CONFIG_PARSED_MOULE__*)_Element_1)->hash;
        uint64_t hash2 = ((struct __CONFIG_PARSED_MOULE__*)_Element_2)->hash;

        if ( hash1 > hash2 ) {
                return 1;
        } else if ( hash1 < hash2 ) {
                return -1;
        }

        return 0;
}

int __Compare_modules(const void *_Module_1, const void *_Module_2) {
        // compares modules hashes
        uint64_t value1 = ((struct __CONFIG_PARSED_MOULE__*)_Module_1)->hash;
        uint64_t value2 = ((struct __CONFIG_PARSED_MOULE__*)_Module_2)->hash;

        if ( value1 > value2 ) {
                return 1;
        } else if ( value1 < value2 ) {
                return -1;
        }

        // compare elements count
        value1 = ((struct __CONFIG_PARSED_MOULE__*)_Module_1)->elements_count;
        value2 = ((struct __CONFIG_PARSED_MOULE__*)_Module_2)->elements_count;

        if ( value1 > value2 ) {
                return 1;
        } else if ( value1 < value2 ) {
                return -1;
        }

        return 0;
}


PARSED_CONFIG Config_Parse(char *_FileName) {
        // open file
        FILE *config_file = fopen(_FileName, "r");
        if ( config_file == NULL ) {
                LogError("Config_Parse", "parse failed: Couldn't open %s", _FileName);
                return NULL_CONFIG;
        }

        PARSED_CONFIG config;
        config.modules = malloc( sizeof(struct __CONFIG_PARSED_MOULE__) );
        if ( config.modules == NULL ) {
                LogError("Config_Parse", "Couldn't open allocate memory for [config]");
                return NULL_CONFIG;
        }
        config.modules_count = 1;
        
        struct __CONFIG_PARSED_MOULE__ *now_module = &config.modules[0];
        now_module->name = "";
        now_module->hash = __hash("");
        now_module->elements_count = 0;

        now_module->elements = malloc( 0 );
        if ( now_module->elements == NULL ) {
                LogError("Config_Parse", "Couldn't open allocate memory for [now_module->elements]");
                return NULL_CONFIG;
        }


        char buffer[1024] = "";
        
        // parsing
        while ( fgets(buffer, sizeof(buffer), config_file) ) {
                int buffer_len = strlen(buffer);
                if ( buffer[ buffer_len - 1 ] == '\n' ) {
                        buffer[ buffer_len - 1 ] = '\0';
                }

                char *now = buffer;
                char *key = NULL;
                char *value = NULL;
                bool bracket_open = 0;
                
                while ( *now == ' ' || *now == '\t' ) {
                        now++;
                }

                if ( *now == '\0' || *now == '#' || *now == ':' ) {
                        continue;
                }

                key = now;
                bracket_open = ( *now == '[' );

                char *last_space = NULL;
                bool prev_is_space = 0;

                while ( *now != ':' && *now != '\0' && ( bracket_open == 0 || *now != ']' ) ) {
                        if ( *now == ' ' ) {
                                if ( prev_is_space == 0 ) {
                                        last_space = now;
                                        prev_is_space = 1;
                                }
                        } else {
                                last_space = NULL;
                                prev_is_space = 0;
                        }

                        now++;
                }

                switch ( *now ) { 
                        case  '\0':
                                continue;
                                break;
                        case ':':
                                if ( last_space ) {
                                        *last_space = '\0';
                                }
                                value = now + 1;
                                *now = '\0';
                                break;
                        case ']':
                                key++;
                                while ( *key == ' ' || *key == '\t' ) {
                                        key++;
                                }
                                if ( last_space ) {
                                        *last_space = '\0';
                                }
                                *now = '\0'; 
                                now_module = __Config_SetModule(&config, key);
                                if ( now_module == NULL ) {
                                        LogError("Config_Parse", "[__Config_SetModule] failed");
                                        now_module = config.modules;
                                }
                                LogTrace("Config_Parse/parsing", "module: \"%s\"\n", key);
                                continue;
                                break;
                        default:
                                continue;
                                break;
                }

                __Module_AddElement(now_module, key, value);
                LogTrace("Config_Parse/parsing", "%s |\"%s\": {%s}\n", now_module->name, key, value);
        }

        qsort(config.modules, config.modules_count, sizeof(struct __CONFIG_PARSED_MOULE__), __Compare_modules);

        for ( int i = 0 ; i < config.modules_count ; i++ ) {
                struct __CONFIG_PARSED_MOULE__ now_module = config.modules[i];
                qsort(now_module.elements, now_module.elements_count, sizeof(struct __CONFIG_PARSED_ELEMENT__), __Compare_elements);
        }
        
        return config;
}



void Config_Print(PARSED_CONFIG _Config) {
        for ( int i = 0 ; i < _Config.modules_count ; i++ ) {
                struct __CONFIG_PARSED_MOULE__ now_module = _Config.modules[i];
                printf("[%s]\n", now_module.name);
                for ( int j = 0 ; j < now_module.elements_count ; j++ ) {
                        struct __CONFIG_PARSED_ELEMENT__ element = now_module.elements[j];
                        printf("%s:%s\n", element.key, element.raw_value);
                }
        }
}




struct __CONFIG_PARSED_MOULE__ *__Config_FindModule(PARSED_CONFIG config, char *_Module_name) {
        int left = 0;
        int right = config.modules_count - 1;
        struct __CONFIG_PARSED_MOULE__ *arr = config.modules;
        uint64_t hash = __hash(_Module_name);

        while (left <= right) {
                int mid = left + (right - left) / 2;

                if (arr[mid].hash == hash) {
                        while ( mid && arr[mid-1].hash == hash ) {
                                mid--;
                        }

                        while ( mid < config.modules_count && arr[mid].hash == hash ) {
                                if ( strcmp(arr[mid].name, _Module_name) == 0 ) {
                                        return &arr[mid];
                                }
                                mid++;
                        }

                        return NULL;
                }

                if (arr[mid].hash < hash) {
                        left = mid + 1;
                } else {
                        right = mid - 1;
                }
        }

        return NULL;
}

struct __CONFIG_PARSED_ELEMENT__ *__Config_FindElement(struct __CONFIG_PARSED_MOULE__ _Module, char *_Element_name) {
        int left = 0;
        int right = _Module.elements_count - 1;
        struct __CONFIG_PARSED_ELEMENT__ *arr = _Module.elements;
        uint64_t hash = __hash(_Element_name);

        while (left <= right) {
                int mid = left + (right - left) / 2;

                if (arr[mid].hash == hash) {
                        while ( mid && arr[mid-1].hash == hash ) {
                                mid--;
                        }

                        while ( mid < _Module.elements_count && arr[mid].hash == hash ) {
                                if ( strcmp(arr[mid].key, _Element_name) == 0 ) {
                                        return &arr[mid];
                                }
                                mid++;
                        }

                        return NULL;
                }

                if (arr[mid].hash < hash) {
                        left = mid + 1;
                } else {
                        right = mid - 1;
                }
        }

        return NULL;
}


CONFIG_VALUE *Config_GetElement(PARSED_CONFIG config, char *_Element_name, char *_Module_name) {
        struct __CONFIG_PARSED_MOULE__ *module = __Config_FindModule(config, _Module_name);
        if ( module == NULL ) {
                return NULL;
        }

        struct __CONFIG_PARSED_ELEMENT__ *element = __Config_FindElement(*module, _Element_name);
        if ( element == NULL ) {
                return NULL;
        }

        return &element->value;
}




void __Config_FreeElement(struct __CONFIG_PARSED_ELEMENT__ _Element) {
        free(_Element.key);
        free(_Element.raw_value);
        if ( _Element.type == VALUE_TYPE_STR ) {
                free(_Element.value.asStr);
        }
}

void __Config_FreeModule(struct __CONFIG_PARSED_MOULE__ _Module) {
        for ( int i = 0 ; i < _Module.elements_count ; i++ ) {
                __Config_FreeElement(_Module.elements[i]);
        }
        free(_Module.elements);
        free(_Module.name);
}


void Config_Delete(PARSED_CONFIG config) {
        for ( int i = 0 ; i < config.modules_count ; i++ ) {
               __Config_FreeModule(config.modules[i]);
        }

        free(config.modules);
}





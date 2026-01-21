#include "config_parser.h"
#include "logs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>



uint64_t __hash1(char *str) {
        uint64_t hash = 0;
        while ( *str ) {
                hash = hash << 8;
                hash |= *str;
                str++;
        }
        return hash;
}

uint64_t __hash2(char *str) {
        return (uint64_t)(*str) * (uint64_t)(*str) * strlen(str);
        
}


int __element_compare(const void *a, const void *b) {
        uint64_t hash_a = ((__CONFIG_PARSED_ELEMENT__*)a)->hash1;
        uint64_t hash_b = ((__CONFIG_PARSED_ELEMENT__*)b)->hash1;
        if ( a < b ) {
                return 1;
        } else if ( a > b ) {
                return -1;
        }

        hash_a = ((__CONFIG_PARSED_ELEMENT__*)a)->hash2;
        hash_b = ((__CONFIG_PARSED_ELEMENT__*)b)->hash2;

        if ( a < b ) {
                return 1;
        } else if ( a > b ) {
                return -1;
        }
        
        return 0;
}

PARSED_CONFIG *Config_Parse(char *file_name) {
        // open source file
        FILE *config_file = fopen(file_name, "r");
        if ( config_file == NULL ) {
                LogError("Config_Parse", "parse failed: Couldn't open %s", file_name);
                return NULL;
        }

        // open file for parsed values
        char parsed_file_path[1024] = ".";
        strcat_s(parsed_file_path, sizeof(parsed_file_path), file_name);
        strcat_s(parsed_file_path, sizeof(parsed_file_path), ".parsed");


        FILE *parsed_file = fopen(parsed_file_path, "w+");
        if ( parsed_file == NULL ) {
                LogError("Config_Parse", "parse failed: Couldn't create %s", parsed_file_path);
                return NULL;
        }


        // creating main struct
        PARSED_CONFIG *config = malloc(sizeof(PARSED_CONFIG));
        if ( config == NULL ) {
               LogError("Config_Parse", "parse failed: Couldn't allocate memory for <PARSED_CONFIG>");
               return NULL;
        }

        __CONFIG_PARSED_ELEMENT__ *parsed_array = malloc(0);
        uint64_t element_count = 0;
        uint64_t char_number = 0;
        
        char buffer[1024] = "";

        fseek(config_file, 0, SEEK_SET);
        while ( fgets(buffer, sizeof(buffer), config_file) ) {
                char *name = buffer;
                char *value = buffer;
                bool is_not_element = 1;

                while ( *value ) {
                        if ( *value == ':' ) {
                                *value = '\0';
                                value++;
                                is_not_element = 0;
                                break;
                        }
                        value++;
                }

                if ( is_not_element ) {
                        continue;
                }

                __CONFIG_PARSED_ELEMENT__ *realloc_result = realloc(parsed_array, sizeof(__CONFIG_PARSED_ELEMENT__)*(element_count+1));
                if ( realloc_result == NULL ) {
                        LogError("Config_Parse", "parse failed: Couldn't reallocate memory for <__CONFIG_PARSED_ELEMENT__>");
                        fclose(config_file);
                        fclose(parsed_file);
                        free(config);
                        return NULL;
                }
                parsed_array = realloc_result;
        
                parsed_array[element_count] = (__CONFIG_PARSED_ELEMENT__){__hash1(name), __hash2(name), char_number};
                char_number += strlen(value) + 1;
        
                fputs(value, parsed_file);
                fflush(parsed_file);
        
                element_count++;
        }

        fclose(config_file);
        config->element_array = parsed_array;
        config->element_count = element_count;
        config->parsed_file = parsed_file;

        qsort(parsed_array, element_count, sizeof(__CONFIG_PARSED_ELEMENT__), __element_compare);
        
        return config;
}





char *Config_GetElement(PARSED_CONFIG *config, char *element_name, char *buffer, int max_count) {
        int left = 0;
        int right = config->element_count - 1;
        int mid = 0;

        uint64_t key = __hash1(element_name);
        __CONFIG_PARSED_ELEMENT__ *array = config->element_array;

        while (left <= right) {
                mid = left + (right - left) / 2;

                if (array[mid].hash1 == key) {
                        goto get_string;
                }

                if (array[mid].hash1 < key) {
                        left = mid + 1;
                } else {
                        right = mid - 1;
                }
        }

        return NULL;

        get_string:
        uint64_t key2 = __hash2(element_name);
        __CONFIG_PARSED_ELEMENT__ *now = &array[mid];

        if ( key2 > now->hash2 ) {
                while ( key2 != now->hash2 && now != array ) {
                        now--;
                        if ( now->hash1 != key ) {
                                return NULL;
                        }
                }
        } else if ( key2 < now->hash2 ) {
                while ( key2 != now->hash2 && now != array + config->element_count - 1 ) {
                        now++;
                        if ( now->hash1 != key ) {
                                return NULL;
                        }
                }    
        }

        fseek(config->parsed_file, now->string_number, SEEK_SET);
        fgets(buffer, max_count, config->parsed_file);
}



void Config_Delete(PARSED_CONFIG *config) {
        if ( config == NULL ) {
                return;
        }

        free(config->element_array);
        free(config);
}


#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H


#include "config_parser.h"
#include "logs.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>



typedef struct _struct_CONFIG_PARSED_ELEMENT {
        uint64_t hash1;
        uint64_t hash2;
        uint64_t string_number;
} __CONFIG_PARSED_ELEMENT__;


typedef struct PARSED_CONFIG {
        __CONFIG_PARSED_ELEMENT__ *element_array;
        uint64_t element_count;
        FILE *parsed_file;
} PARSED_CONFIG;


PARSED_CONFIG *Config_Parse(char *file_name);
char *Config_GetElement(PARSED_CONFIG* config, char *element_name, char *buffer, int max_count);
void Config_Delete(PARSED_CONFIG* config);





#endif // CONFIG_PARSER_H
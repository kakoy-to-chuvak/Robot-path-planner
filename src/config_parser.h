#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H


#include "config_parser.h"
#include "logs.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#define NULL_CONFIG (PARSED_CONFIG){NULL, 0}


typedef union __union_CONFIG_VALUE__ {
        char      *raw;
        char      *asStr;
        int64_t   asInt;
        double    asFloat;
        SDL_Color asColor;
} CONFIG_VALUE;

typedef enum __enum_value_types__ {
        VALUE_TYPE_RAW,
        VALUE_TYPE_STR,
        VALUE_TYPE_INT,
        VALUE_TYPE_FLOAT,
        VALUE_TYPE_COLOR
} CONFIG_VALUE_TYPE;



struct __CONFIG_PARSED_ELEMENT__ {
        uint64_t          hash;
        char*             key;
        char*             raw_value;
        CONFIG_VALUE      value;
        CONFIG_VALUE_TYPE type;
};


struct __CONFIG_PARSED_MOULE__ {
        uint64_t hash;
        char *name;
        struct __CONFIG_PARSED_ELEMENT__ *elements;
        int elements_count;
};


typedef struct _struct_PARSED_CONFIG {
      struct __CONFIG_PARSED_MOULE__ *modules;
      int modules_count;  
} PARSED_CONFIG;


PARSED_CONFIG Config_Parse(char *file_name);
void Config_Print(PARSED_CONFIG _Config);
CONFIG_VALUE *Config_GetElement(PARSED_CONFIG config, char *_Element_name, char *_Module_name);
void Config_Delete(PARSED_CONFIG config);





#endif // CONFIG_PARSER_H
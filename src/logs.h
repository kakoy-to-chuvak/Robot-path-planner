#ifndef LOGS_H
#define LOGS_H

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#if defined _WIN32 || defined _WIN64
#	include <windows.h>
#else
#	include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_TRACE     (1)
#define LOG_LEVEL_DEBUG     (2)
#define LOG_LEVEL_NOTICE    (3)
#define LOG_LEVEL_INFO      (4)
#define LOG_LEVEL_WARN      (5)
#define LOG_LEVEL_ERROR     (6)


#define LOG_COLOR_Black           ((LOG_COLOR)(30))  
#define LOG_COLOR_Red             ((LOG_COLOR)(31))  
#define LOG_COLOR_Green           ((LOG_COLOR)(32))  
#define LOG_COLOR_Yellow          ((LOG_COLOR)(33))  
#define LOG_COLOR_Blue            ((LOG_COLOR)(34))  
#define LOG_COLOR_Magenta         ((LOG_COLOR)(35))  
#define LOG_COLOR_Cyan            ((LOG_COLOR)(36))  
#define LOG_COLOR_White           ((LOG_COLOR)(37))  
#define LOG_COLOR_Bright_Black    ((LOG_COLOR)(90))  
#define LOG_COLOR_Bright_Red      ((LOG_COLOR)(91))  
#define LOG_COLOR_Bright_Green    ((LOG_COLOR)(92))  
#define LOG_COLOR_Bright_Yellow   ((LOG_COLOR)(93))  
#define LOG_COLOR_Bright_Blue     ((LOG_COLOR)(94))  
#define LOG_COLOR_Bright_Magenta  ((LOG_COLOR)(95))  
#define LOG_COLOR_Bright_Cyan     ((LOG_COLOR)(96))  
#define LOG_COLOR_Bright_White    ((LOG_COLOR)(97))  

typedef unsigned short LOG_COLOR;

// Основные функции логирования
int __Logs_LogArgs(const char* level, LOG_COLOR color, const char* module, const char* file, int line, const char* message_format, va_list args);
int Logs_LogCustom(const char* level, LOG_COLOR color, const char* module, const char* file, int line, const char* mesage_format, ...);
int Logs_Log(int level, const char* module, const char* file, int line, const char* mesage_format, ...);


// Макросы для удобства
#define Log(level, module, fmt, ...)                Logs_Log(level, module, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LogError(module, fmt, ...)                  Log(LOG_LEVEL_ERROR,  module, fmt, ##__VA_ARGS__)
#define LogWarn(module, fmt, ...)                   Log(LOG_LEVEL_WARN,   module, fmt, ##__VA_ARGS__)
#define LogInfo(module, fmt, ...)                   Log(LOG_LEVEL_INFO,   module, fmt, ##__VA_ARGS__)
#define LogNotice(module, fmt, ...)                 Log(LOG_LEVEL_NOTICE, module, fmt, ##__VA_ARGS__)
#define LogDebug(module, fmt, ...)                  Log(LOG_LEVEL_DEBUG,  module, fmt, ##__VA_ARGS__)
#define LogTrace(module, fmt, ...)                  Log(LOG_LEVEL_TRACE,  module, fmt, ##__VA_ARGS__)
#define LogCustom(level, color, module, fmt, ...)   Logs_LogCustom(level, color, module, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define Log_wf(level, module, fmt, ...)                Logs_Log(level, module, NULL, 0, fmt, ##__VA_ARGS__)
#define LogError_wf(module, fmt, ...)                  Log_wf(LOG_LEVEL_ERROR,  module, fmt, ##__VA_ARGS__)
#define LogWarn_wf(module, fmt, ...)                   Log_wf(LOG_LEVEL_WARN,   module, fmt, ##__VA_ARGS__)
#define LogInfo_wf(module, fmt, ...)                   Log_wf(LOG_LEVEL_INFO,   module, fmt, ##__VA_ARGS__)
#define LogNotice_wf(module, fmt, ...)                 Log_wf(LOG_LEVEL_NOTICE, module, fmt, ##__VA_ARGS__)
#define LogDebug_wf(module, fmt, ...)                  Log_wf(LOG_LEVEL_DEBUG,  module, fmt, ##__VA_ARGS__)
#define LogTrace_wf(module, fmt, ...)                  Log_wf(LOG_LEVEL_TRACE,  module, fmt, ##__VA_ARGS__)
#define LogCustom_wf(level, color, module, fmt, ...)   Logs_LogCustom(level, color, module, NULL, 0, fmt, ##__VA_ARGS__)



// API
void Logs_SetLogLevel(int level);
void Logs_EnableLogs(int enable);
void Logs_EnableColors(int enable);
// Files 
void Logs_CloseFile(void);          
FILE* Logs_SetFile_name(const char* filename);
FILE* Logs_SetFile_stream(FILE* stream);
#define Logs_SetFile(file) _Generic((file), \
                            FILE* : Logs_SetFile_stream, \
                            char* : Logs_SetFile_name, \
                            default : 0)(file)


#ifdef __cplusplus
}
#endif

#endif // LOGS_H
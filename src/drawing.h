#ifndef _DRAWING_H_
#define _DRAWING_H_ 


#include <SDL3/SDL.h>

#include "include.h"
#include "vectors.h"



void RenderLine(SDL_Renderer *renderer, SDL_FPoint P0, SDL_FPoint P1, int32_t radius);
void RenderArrow(SDL_Renderer *renderer, SDL_FPoint P0, SDL_FPoint P1, double base, int32_t P1_radius);
void RenderCircle(SDL_Renderer *renderer, int32_t x0, int32_t y0, int32_t radius);
void Render_RounderRect(SDL_Renderer *renderer, SDL_FRect rect, int32_t radius);
void RenderCross(SDL_Renderer *_Renderer, SDL_FPoint _Center);
void RenderVector(SDL_Renderer *_Renderer, SDL_FPoint _Start, SDL_FPoint _End, int32_t width, int32_t arrow_width);


#endif // _DRAWING_H_
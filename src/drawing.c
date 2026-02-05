#include "drawing.h"


void RenderLine(SDL_Renderer *renderer, SDL_FPoint P0, SDL_FPoint P1, int32_t radius) {
        int32_t x = radius;
        int32_t y = 0; 
        int32_t err = 0;

        int32_t x0 = P0.x;
        int32_t y0 = P0.y;
        int32_t x1 = P1.x;
        int32_t y1 = P1.y;

        while ( x > y ) {
                if (err <= 0) {
                        y += 1;
                        err += 2*y + 1;
        	} else {
                        x -= 1;
                        err -= 2*x + 1;
        	}
                
                SDL_RenderLine(renderer,  x0 + x - 1,  y0 + y - 1,  x1 + x - 1,  y1 + y - 1 );
                SDL_RenderLine(renderer,  x0 + x - 1,  y0 - y    ,  x1 + x - 1,  y1 - y     );
                SDL_RenderLine(renderer,  x0 - x    ,  y0 + y - 1,  x1 - x    ,  y1 + y - 1 );
                SDL_RenderLine(renderer,  x0 - x    ,  y0 - y    ,  x1 - x    ,  y1 - y     );
                SDL_RenderLine(renderer,  x0 + y - 1,  y0 + x - 1,  x1 + y - 1,  y1 + x - 1 );
                SDL_RenderLine(renderer,  x0 + y - 1,  y0 - x    ,  x1 + y - 1,  y1 - x     );
                SDL_RenderLine(renderer,  x0 - y    ,  y0 + x - 1,  x1 - y    ,  y1 + x - 1 );
                SDL_RenderLine(renderer,  x0 - y    ,  y0 - x    ,  x1 - y    ,  y1 - x     );
        }
}


void RenderArrow(SDL_Renderer *renderer, SDL_FPoint P0, SDL_FPoint P1, double base, int32_t P1_radius) {
        SDL_FPoint Pa = Vector_Norm( Vector_Sub(P1, P0) );

        P1 = Vector_Sub(P1, Vector_Mult_scl(Pa, P1_radius + base));

        SDL_Vertex vertex[3];

        SDL_FColor arrow_color;
        SDL_GetRenderDrawColorFloat(renderer, &arrow_color.r, &arrow_color.g, &arrow_color.b, &arrow_color.a);

        vertex[0].color = arrow_color;
        vertex[1].color = arrow_color;
        vertex[2].color = arrow_color;

        Pa = Vector_Mult_scl(Pa, base);
        vertex[0].position = Vector_Sum(P1, Pa);

        Pa = Vector_Rotate90(Pa);
        vertex[1].position = Vector_Sum(P1, Pa);

        Pa = Vector_Invert(Pa);
        vertex[2].position = Vector_Sum(P1, Pa);


        SDL_RenderGeometry(renderer, NULL, vertex, 3, NULL, 0);
}


void RenderCircle(SDL_Renderer *renderer, int32_t x0, int32_t y0, int32_t radius) {
        int32_t x = 0;
        int32_t y = radius;
        int32_t p = 1 - radius;

        SDL_RenderPoint(renderer, x0 + x, y0 + y);
        SDL_RenderPoint(renderer, x0 + y, y0 + x);
        SDL_RenderPoint(renderer, x0 - y, y0 + x);
        SDL_RenderPoint(renderer, x0 - x, y0 + y);
        SDL_RenderPoint(renderer, x0 - x, y0 - y);
        SDL_RenderPoint(renderer, x0 - y, y0 - x);
        SDL_RenderPoint(renderer, x0 + y, y0 - x);
        SDL_RenderPoint(renderer, x0 + x, y0 - y);

        while (x < y)
        {       
                
                if (p < 0){
                        p += 2 * x + 1;
                        x++;
                }else{
                        p += 2 * (x - y) + 1;
                        y--;
                        x++;
                }

                SDL_RenderPoint(renderer, x0 + x, y0 + y);
        	SDL_RenderPoint(renderer, x0 + y, y0 + x);
        	SDL_RenderPoint(renderer, x0 - y, y0 + x);
        	SDL_RenderPoint(renderer, x0 - x, y0 + y);
        	SDL_RenderPoint(renderer, x0 - x, y0 - y);
        	SDL_RenderPoint(renderer, x0 - y, y0 - x);
        	SDL_RenderPoint(renderer, x0 + y, y0 - x);
        	SDL_RenderPoint(renderer, x0 + x, y0 - y);

                
        }
}


void Render_RounderRect(SDL_Renderer *renderer, SDL_FRect rect, int32_t radius) {
        
        SDL_FRect now_rect = {
                rect.x,
                rect.y + radius,
                rect.w,
                rect.h - 2 * radius
        };

        SDL_RenderFillRect(renderer, &now_rect);

        if ( radius )
                radius--;
                
        int32_t x_l = rect.x + radius;
        int32_t y_u = rect.y + radius;
        int32_t x_r = rect.x + rect.w - radius - 1;
        int32_t y_b = rect.y + rect.h - radius - 1;

        int32_t x = 0;
        int32_t y = radius;
        int32_t p = 1 - radius;

        int32_t y_next = p < 0 ? y : y - 1;
        int32_t p_next = p < 0 ? p + 2 * x + 1 : p + 2 * ( x - y ) + 1;
        
        SDL_RenderLine(renderer, x_l - y, y_u - x, x_r + y, y_u - x);
        SDL_RenderLine(renderer, x_l - y, y_b + x, x_r + y, y_b + x);
        if ( y != y_next ) {      
                SDL_RenderLine(renderer, x_l - x, y_u - y, x_r + x, y_u - y);
                SDL_RenderLine(renderer, x_l + x, y_b - y, x_r + x, y_b + y);
        }
                
        while (x < y)
        {
        	p = p_next;
                y = y_next;
                x++;

                y_next = p < 0 ? y : y - 1;
                p_next = p < 0 ? p + 2 * x + 1 : p + 2 * ( x - y ) + 1;

                if ( x != y ) {
                        SDL_RenderLine(renderer, x_l - y, y_u - x, x_r + y, y_u - x);
                        SDL_RenderLine(renderer, x_l - y, y_b + x, x_r + y, y_b + x);
                }

                if ( y != y_next ) {
                        SDL_RenderLine(renderer, x_l - x, y_u - y, x_r + x, y_u - y);
                        SDL_RenderLine(renderer, x_l - x, y_b + y, x_r + x, y_b + y);
                }
                
        }
}




void RenderCross(SDL_Renderer *_Renderer, SDL_FPoint _Center) {
        SDL_RenderLine(_Renderer, _Center.x, 0, _Center.x, 10000);
        SDL_RenderLine(_Renderer, 0, _Center.y, 10000, _Center.y);
        SDL_RenderLine(_Renderer, _Center.x - 10000, _Center.y - 10000, _Center.x + 10000, _Center.y + 10000);
        SDL_RenderLine(_Renderer, _Center.x - 10000, _Center.y + 10000, _Center.x + 10000, _Center.y - 10000);
}



void RenderVector(SDL_Renderer *_Renderer, SDL_FPoint _Start, SDL_FPoint _End, int32_t width, int32_t arrow_width) {
        RenderLine(_Renderer, _Start, _End, width);
        RenderArrow(_Renderer, _Start, _End, arrow_width, -2*width);
}
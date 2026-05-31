#include "vectors.h"
#include <stdio.h>

SDL_FPoint Vector_Sum(SDL_FPoint P1, SDL_FPoint P2) {
        return (SDL_FPoint){ 
                P1.x + P2.x, 
                P1.y + P2.y 
        };
}

SDL_FPoint Vector_Sub(SDL_FPoint P1, SDL_FPoint P2) {
        return (SDL_FPoint){ 
                P1.x - P2.x, 
                P1.y - P2.y 
        };
}

double Vector_DotProd(SDL_FPoint P1, SDL_FPoint P2) {
        return P1.x * P2.x + P1.y * P2.y;
}

SDL_FPoint Vector_Mult_scl(SDL_FPoint P1, double k) {
        return (SDL_FPoint){
                P1.x * k, 
                P1.y * k 
        };
}

SDL_FPoint Vector_Divscl(SDL_FPoint P1, double k) {
        return (SDL_FPoint){ 
                P1.x / k, 
                P1.y / k 
        };
}

double Vector_Abs(SDL_FPoint P) {
        return sqrt( P.x * P.x + P.y * P.y );
}

double Vector_SqAbs(SDL_FPoint P) {
        return P.x * P.x + P.y * P.y;
}

SDL_FPoint Vector_Norm(SDL_FPoint P) {
        return Vector_Divscl(P, Vector_Abs(P));
}

double Vector_Cos(SDL_FPoint P1, SDL_FPoint P2) {
        return Vector_DotProd(P1, P2) / ( Vector_Abs(P1) * Vector_Abs(P2) );
}

double Vector_Angle(SDL_FPoint P) {
        if ( P.x || P.y ) {
                float cos_a = Vector_Cos(P, (SDL_FPoint){1, 0});
                if ( cos_a < -1 ) {
                        return M_PI;
                } else if ( cos_a > 1 ) {
                        return 0; 
                }

                if ( P.y > 0 ) {
                        return acos( cos_a );
                } else {
                        return -acos( cos_a );
                }
        }
        return 0;
}


SDL_FPoint Vector_Rotate(SDL_FPoint P, double rad) {
        double _Cos = cos(rad);
        double _Sin = sin(rad);

        return (SDL_FPoint){
             _Cos * P.x + _Sin * P.y,
             _Cos * P.y - _Sin * P.x    
        };
}

SDL_FPoint Vector_Rotate180(SDL_FPoint P) {
        return (SDL_FPoint) {
                -P.x,
                -P.y
        };
}

SDL_FPoint Vector_Rotate90(SDL_FPoint P) {
        return (SDL_FPoint) {
                -P.y,
                P.x
        };
}

SDL_FPoint Vector_Rotate90m(SDL_FPoint P) {
        return (SDL_FPoint) {
                P.y,
                -P.x
        };
}

SDL_FPoint Vector_Invert(SDL_FPoint P) {
        return (SDL_FPoint) {
                -P.x,
                -P.y
        };
}

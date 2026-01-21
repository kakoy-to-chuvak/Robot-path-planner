#include "label.h"

void BlitSymetricalPoints(SDL_Surface *src, SDL_Surface *dst, int radius, int x, int y) {
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius+x,radius+y,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius-x,radius+y,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius+x,radius-y,0,0}) );
 	SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius-x,radius-y,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius+y,radius+x,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius-y,radius+x,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius+y,radius-x,0,0}) );
        SDL_BlitSurface(src, NULL, dst, &((SDL_Rect){radius-y,radius-x,0,0}) );
}


SDL_Surface *RenderBubbleText(TTF_Font *font, const char *_Text, size_t length, int radius, SDL_Color inner_color, SDL_Color outer_color) {
        SDL_Surface *outer_surf = TTF_RenderText_Blended(font, _Text, length, outer_color);
        if ( NULL == outer_surf ) {
                return NULL;
        }

        SDL_Surface *inner_surf = TTF_RenderText_Blended(font, _Text, length, inner_color);
        if ( NULL == inner_surf ) {
                SDL_DestroySurface(outer_surf);
                return NULL;
        }

        SDL_Surface *result_surf = SDL_CreateSurface(outer_surf->w + 2 * radius, outer_surf->h + 2 * radius, outer_surf->format);
        if ( NULL == result_surf ) {
                SDL_DestroySurface(outer_surf);
                SDL_DestroySurface(inner_surf);
                return NULL;
        }

        int x = 0, y = radius;
        int d = 3 - 2 * radius;
        BlitSymetricalPoints(outer_surf, result_surf, radius, x, y);
        while (y >= x){
                if (d > 0) {
                        y--; 
                        d = d + 4 * (x - y) + 10;
                }
                else
                        d = d + 4 * x + 6;

                x++;
                BlitSymetricalPoints(outer_surf, result_surf, radius, x, y);
        }

        SDL_BlitSurface(inner_surf, NULL, result_surf, &((SDL_Rect){radius,radius,0,0}));

        SDL_DestroySurface(outer_surf);
        SDL_DestroySurface(inner_surf);

        return result_surf;
}


SDL_Surface *RenderBorderedText(TTF_Font *font, const char *_Text, size_t length, int radius, SDL_Color inner_color, SDL_Color outer_color) {

        SDL_Surface *outer_surf = TTF_RenderText_Blended(font, _Text, length, outer_color);
        if ( NULL == outer_surf ) {
                return NULL;
        }

        SDL_Surface *inner_surf = TTF_RenderText_Blended(font, _Text, length, inner_color);
        if ( NULL == inner_surf ) {
                SDL_DestroySurface(outer_surf);
                return NULL;
        }

        SDL_Surface *result_surf = SDL_CreateSurface(outer_surf->w + 2 * radius, outer_surf->h + 2 * radius, outer_surf->format);
        if ( NULL == result_surf ) {
                SDL_DestroySurface(outer_surf);
                SDL_DestroySurface(inner_surf);
                return NULL;
        }

        int x = 0;
        BlitSymetricalPoints(outer_surf, result_surf, radius, x, radius);
        while (radius-1 >= x){
                x++;
                BlitSymetricalPoints(outer_surf, result_surf, radius, x, radius);
        }

        SDL_BlitSurface(inner_surf, NULL, result_surf, &((SDL_Rect){radius,radius,0,0}));

        SDL_DestroySurface(outer_surf);
        SDL_DestroySurface(inner_surf);

        return result_surf;
}


LABEL *Label_New(SDL_Renderer *renderer, const char *font_name, const char *_Text, float ptsize, SDL_Color foreground, int16_t flags, LABEL_PARAMS params) {
        // null pointer check
        if ( renderer == 0 || font_name == 0 || _Text == 0 )
                return NULL;
        
        // allocate memory for struct
        LogTrace("Label_New", "malloc: allocating memory for new label");
        LABEL *text = (LABEL*)malloc(sizeof(LABEL));
        if ( 0 == text ) {
                LogError("TextNew", "Error on allocate memory");
                goto failure0;
        }
        // open font
        LogTrace("Label_New", "TTF_OpenFont: open font <%s>", font_name);
        TTF_Font *font = TTF_OpenFont(font_name, ptsize);
        if ( font == NULL ) {
                LogError("TextNew", "Error on TTF_OpenFont: %s", SDL_GetError());
                goto failure1;
        }
        // rendering text
        LogTrace("Label_New", "<SDL_Surface *text_surface>: creating <text_surface>");
        SDL_Surface *text_surface;
        if ( flags & LABEL_PARAM_BUBBLE )
                text_surface = RenderBubbleText(font, _Text, 0, params.radius, foreground, params.outer_color);
        else if (flags & LABEL_PARAM_BORDER ) 
                text_surface = RenderBorderedText(font, _Text, 0, params.radius, foreground, params.outer_color);
        else
                text_surface = TTF_RenderText_Blended(font, _Text, 0, foreground);

        if ( 0 == text_surface ) {
                LogError("TextNew", "Error on TTF_RenderText_Blended: %s", SDL_GetError());
                goto failure2;
        }
        // creating texture
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

        text->rect.w = (float)text_surface->w;
        text->rect.h = (float)text_surface->h;

        LogTrace("Label_New", "SDL_DestroySurface: destroying <text_surface>");
        SDL_DestroySurface(text_surface);
        if ( 0 == text_texture ) {
                LogError("TextNew", "Error on SDL_CreateTextureFromSurface: %s", SDL_GetError());
                goto failure2;
        }

        text->renderer = renderer;
        text->font = font;
        text->texture = text_texture;
        text->params = params;
        text->params_flags = flags;
        return text;

        failure2:
        TTF_CloseFont(font);
        failure1:
        free(text);
        failure0:
        LogTrace("Label_New", "fail");
        return NULL;
}


int Label_Free(LABEL *label) {
        if ( NULL==label )
                return 0;
        if (label->texture) SDL_DestroyTexture(label->texture);
        if (label->font) TTF_CloseFont(label->font);
        free(label);   
        return 1; 
}


bool Label_Draw(LABEL *label, SDL_FRect *srcrext, SDL_FRect *dstrect) {
        if ( label && label->texture ) {
                return SDL_RenderTexture(label->renderer, label->texture, srcrext, dstrect);
        }
        return 0;
}


int Label_Update(LABEL *label, const char *_Text, SDL_Color fg) {
        if ( label->texture )
                SDL_DestroyTexture(label->texture);
        SDL_Surface *text_surface;
        if ( label->params_flags & LABEL_PARAM_BUBBLE )
                text_surface = RenderBubbleText(label->font, _Text, 0, label->params.radius, fg, label->params.outer_color);
        else if ( label->params_flags & LABEL_PARAM_BORDER ) 
                text_surface = RenderBorderedText(label->font, _Text, 0, label->params.radius, fg, label->params.outer_color);
        else
                text_surface = TTF_RenderText_Blended(label->font, _Text, 0, fg);
        if ( 0 == text_surface ) 
                return 0;
                
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(label->renderer, text_surface);
        SDL_DestroySurface(text_surface);
        if ( 0 == text_texture ) 
                return 0;

        label->texture = text_texture;
        label->rect.h = text_texture->h;
        label->rect.w = text_texture->w;
        return 1;
}
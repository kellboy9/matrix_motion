#ifndef DIALOGUE_H
#define DIALOGUE_H
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

void load_font(char *filename, SDL_Renderer *renderer);
void draw_message(const char *message, SDL_Renderer *renderer);

#endif

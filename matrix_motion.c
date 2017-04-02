#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "map.h"
#include "calc.h"
#include "entity.h"

//TODO:
//implement dialogue system, handled by engine but called by scripts via a static msg() func or something
//implement view, UI, Physics

//basic in-game objects:
//Map
//View
//Entity
//Dialogue
//UI
//Physics {Static, Dynamic}


int main(int argc, char** argv)
{

	//graphics stuffs
	printf("Initializing graphics objects.\n");
	if(!SDL_Init(SDL_INIT_EVERYTHING)) {
		printf("SDL init fail! SDL_Error: %s\n", SDL_GetError());
	}

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	if(!window) {
		printf("Window creation fail! SDL_Error: %s\n", SDL_GetError());
	}
	if(!renderer) {
		printf("Renderer creation fail! SDL_Error: %s\n", SDL_GetError());
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); 
	//SDL_RenderSetLogicalSize(renderer, 240, 120); 
	SDL_RenderSetLogicalSize(renderer, 640, 400);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//load game objects
	printf("Loading game objects.\n");
	Map *map = new_map("objects/mat", renderer);
	
	printf("Starting main loop.\n");
	//main loop
	int quit = 0;
	while(!quit) {
		//input loop
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				quit = 1;
			}
			else if(e.type == SDL_KEYDOWN) {
				if(e.key.keysym.sym == SDLK_q) {
					quit = 1;
				}
			}
			dispatch_events(e);
		}

		update_components();

		//rendering

		//clear screen
		SDL_SetRenderDrawColor(renderer, 0x88, 0xFF, 0xBB, 0xFF);
		SDL_RenderClear(renderer);

		draw_map(map, renderer);

		SDL_RenderPresent(renderer);
	}

	//free everything
	free_map(map);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

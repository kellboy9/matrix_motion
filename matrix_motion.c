#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "map.h"
#include "calc.h"
#include "entity.h"

//TODO:
//replace rects with images and draw them
//Move to script-based actions
//implement dialogue system, handled by engine but called by scripts via a static msg() func or something

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
	Point a = locate_char(map, 'a');
	Point b = locate_char(map, 'b');
	Point *path = NULL;
	
	printf("Starting main loop.\n");
	//main loop
	int quit = 0;
	while(!quit) {
		//input loop
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			dispatch_events(e);
			if(e.type == SDL_QUIT) {
				quit = 1;
			}
			else if(e.type == SDL_KEYDOWN) {
				if(e.key.keysym.sym == SDLK_q) {
					quit = 1;
				}
				if(e.key.keysym.sym == SDL_GetKeyFromName("Up")) {
					a.y--;
				}
				else if(e.key.keysym.sym == SDLK_DOWN) {
					a.y++;
				}
				else if(e.key.keysym.sym == SDLK_LEFT) {
					a.x--;
				}
				else if(e.key.keysym.sym == SDLK_RIGHT) {
					a.x++;
				}
				else if(e.key.keysym.sym == SDL_GetKeyFromName("P")) {
					if(path)
						free(path);
					path = get_shortest_path(a, b, map);	
				}
				else if(e.key.keysym.sym == SDLK_x) {
					//rebuild
					if(path) {
						for(int i = 0; i < width(map) * height(map); ++i) {
							path[i].x = -1;
							path[i].y = -1;
						}
					}
					free_map(map);
					map = new_map("objects/mat", renderer);
					a = locate_char(map, 'a');
					b = locate_char(map, 'b');
				}
			}
		}

		update_components();

		//rendering

		//clear screen
		SDL_SetRenderDrawColor(renderer, 0x88, 0xFF, 0xBB, 0xFF);
		SDL_RenderClear(renderer);

		draw_map(map, renderer);

		if(path) {
			for(int k = 0; k < width(map) * height(map); ++k) {
				if(path[k].x == -1) {
					break;
				} else {
					SDL_Rect pos = {path[k].x*11, path[k].y*11, 11, 11};
					SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0x88);
					SDL_RenderFillRect(renderer, &pos);
				}
			}
		}

		SDL_Rect a_rect = {a.x*11, a.y*11, 11, 11};
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(renderer, &a_rect);
		SDL_Rect b_rect = {b.x*11, b.y*11, 11, 11};
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
		SDL_RenderFillRect(renderer, &b_rect);

		SDL_RenderPresent(renderer);
	}

	//free everything
	free_map(map);
	if(path)
		free(path);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

#ifndef ENTITY_H
#define ENTITY_H
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

//module for managing the entities + handling their events
typedef struct entity Entity;
struct entity {
	char *name; //used for lookup
	SDL_Texture *draw; //graphic to draw (in memory) TODO: read from db, not local
	SDL_Rect pos; //x, y, w, h (dst rect)
	SDL_Rect anim; //animation frame (src rect from draw)
	List *components; //list of components to check for, of type Component
};

typedef struct event Event;
List *register_inputs(char *filename);
void dispatch_events(SDL_Event e);
void update_components();
Entity *new_entity(char *name, SDL_Renderer *renderer); 
Entity *new_entity_pos(char *name, SDL_Renderer *renderer, int x, int y);
void init_entities();

#endif

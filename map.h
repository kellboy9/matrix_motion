#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "calc.h"
#include "list.h"
#include "entity.h"

typedef struct map Map;

Map *new_map(const char *filename, SDL_Renderer *renderer);
void free_map(Map *map);
void draw_map(Map *map, SDL_Renderer *renderer);
//accessor functions since struct is private
size_t width(Map *map);
size_t height(Map *map);
//function functions
Point locate_char(Map *map, char key);
Point *get_shortest_path(Point a, Point b, Map *map);

#endif

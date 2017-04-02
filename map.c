#include "map.h"

struct map {
	size_t width;
	size_t height;
	char **collision;
	int **tiles; //object creation handled by parent, drawing
	SDL_Texture *tiles_src;
	List *entities;
};

Map *new_map(const char* name, SDL_Renderer *renderer) {
	//TODO: input CHEKCING
	char filename[256];
	strcpy(filename, name);
	strcat(filename, ".map");
	FILE *mapfile = fopen(filename, "r");
	Map *map = malloc(sizeof(Map)); 
	char throwaway[16];

	printf("Loading tiles.\n");
	SDL_Surface *tiles_s = SDL_LoadBMP("objects/tiles.bmp");
	if(!tiles_s) {
		printf("Loading objects/tiles.bmp fail! SDL_Error: %s\n", SDL_GetError());
	}
	SDL_SetColorKey(tiles_s, SDL_TRUE, SDL_MapRGB(tiles_s->format, 0xFF, 0x00, 0xFF));
	map->tiles_src = SDL_CreateTextureFromSurface(renderer, tiles_s);
	if(!map->tiles_src) {
		printf("creating tiles texture fail! SDL_Error: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(tiles_s);

	printf("Reading map data.\n");

	char wh[16]; //Width Height
	fgets(wh, 16, mapfile);
	sscanf(wh,"%zu\n", &(map->width));
	fgets(wh, 16, mapfile);
	sscanf(wh,"%zu\n", &(map->height));
	
	char line[map->width];

	//collision map
	printf("Getting collision map.\n");
	fgets(throwaway, sizeof(throwaway), mapfile);
	map->collision = calloc(sizeof(char*), map->height);
	for(int i = 0; i < map->height; ++i) {
		map->collision[i] = calloc(sizeof(char), map->width);
	}
	for(int i = 0; i < map->height; ++i) {
		fgets(line, map->width, mapfile); //read the chars
		fgets(throwaway, sizeof(throwaway), mapfile); //skip the rest
		printf("%s\n", line);
		memcpy(map->collision[i], line, map->width);
	}

	char tiles_line[map->width * 4]; //max 3 digit numbers possible + space

	//object map
	printf("Fetching objects.\n");
	fgets(throwaway, sizeof(throwaway), mapfile); //object
	map->tiles = calloc(sizeof(int*), map->height);
	for(int i = 0; i < map->height; ++i) {
		map->tiles[i] = calloc(sizeof(int), map->width);
	}
	for(int i = 0; i < map->height; ++i) {
		fgets(tiles_line, map->width*4, mapfile); //read the chars
		char *end = tiles_line; //so that it goes from number to number
		for(int j = 0; j < map->width; ++j) {
			map->tiles[i][j] = strtol(end, &end, 10);
		}
	}

	for(int i = 0; i < map->height; ++i) {
		for(int j = 0; j < map->width; ++j) {
			printf("%d", map->tiles[i][j]);
		}
		printf("\n");
	}

	fclose(mapfile);
	printf("Loading entities.\n");
	init_entities();
	map->entities = new_list();
	//TODO: check this input
	char entfilename[256];	
	strcpy(entfilename, name);
	strcat(entfilename, ".entities");
	FILE *entfile = fopen(entfilename, "r");
	char entline[512];
	while(fgets(entline, 512, entfile)) {
		printf("Loading entity... %s\n", entline);
		char ent_name[256];
		int ent_x, ent_y;
		sscanf(entline, "%s %d %d", ent_name, &ent_x, &ent_y);
		printf("Confirm: %s at %d,%d\n", ent_name, ent_x, ent_y);
		add_item(map->entities, new_entity(ent_name, renderer, ent_x, ent_y));
	}
	fclose(entfile);
	
	printf("Done loading map.\n");
	return map;
}

void free_map(Map *map) {
	for(int i = 0; i < map->height; ++i) {
		free(map->collision[i]);
	}
	free(map->collision);
	for(int i = 0; i < map->height; ++i) {
		free(map->tiles[i]);
	}
	free(map->tiles);
	SDL_DestroyTexture(map->tiles_src);
	free_entities(map->entities);
	free(map);
}

void draw_map(Map *map, SDL_Renderer *renderer) {
	for(int i = 0; i < map->height; ++i) {
		for(int j = 0; j < map->width; ++j) {
			SDL_Rect dst = {j*11, i*11, 11, 11};
			SDL_Rect src = {map->tiles[i][j]*11, 0, 11, 11};
			SDL_RenderCopy(renderer, map->tiles_src, &src, &dst);
		}
	}

	Node *i = NULL;
	while(i = next(map->entities, i)) {
		draw_entity(item(i), renderer);
	}
}

size_t width(Map *map) {
	return map->width;
}
size_t height(Map *map) {
	return map->height;
}

Point locate_char(Map *map, char key) {
	int i;
	int j;
	Point located = {-1, -1};
	for(i = 0; i < map->height; ++i) {
		for(j = 0; j < map->width; ++j) {
			if(map->collision[i][j] == key) {
				map->collision[i][j] = '.';
				located.x = j;
				located.y = i;
				return located;
			}
		}
	}
	return located;
}

//operates via collision map aka collision
//moves from a to b
//solution must be freed afterward
Point *get_shortest_path(Point a, Point b, Map *map) {
	
	//pair of stacks for the solution sets
	Point *solution = malloc(sizeof(Point) * width(map) * height(map)); 
	for(int i = 0; i < width(map) * height(map); ++i) {
		solution[i].x = -1;
		solution[i].y = -1;
	}
	int sk = 0;

	Point nosol[width(map) * height(map)]; //nonsolution set (regular array bc local)
	memset(nosol, 0, sizeof(nosol));
	int nk = 0;
	
	//start at a
	Point c = a;
	while(!(c.x == b.x && c.y == b.y)) {
		//add current tile to the nonsolutions list
		nosol[nk++] = c;
		
		//check all adjacent moves (4 at max)
		Point pos[4] = {{c.x-1, c.y}, {c.x+1, c.y}, {c.x, c.y-1}, {c.x, c.y+1}}; //possible paths
		int sd = width(map)*height(map); //shortest distance

		//find which distance is physically closest, add to the solution set
		int i, si = 0;
		for(i = 0; i < 4; ++i) {
			//check against nonsolutions 
			//if it's already a nonsolution, if it's a wall, or if it's out of bounds, don't go to it
			int nosolution = 0, ni;
			if(pos[i].y >= height(map) || pos[i].y < 0 || pos[i].x >= width(map) || pos[i].x < 0) {
				nosolution = 1;
			}
			else if(map->collision[pos[i].y][pos[i].x] == '#') {
				nosolution = 1;
			}
			for(ni = 0; ni < width(map)*height(map); ++ni) {
				if(pos[i].x == nosol[ni].x && pos[i].y == nosol[ni].y) {
					nosolution = 1;
					break;
				}
			}

			if(!nosolution) {
				int d = distance(pos[i], b); //TODO: + tile_weight
				if(d < sd) {
					sd = d;	
					si = i;
				}
			} else {
				nosol[nk++] = pos[i];
			}
		}

		solution[sk++] = pos[si]; //next solution is the shortest current path
		c = pos[si]; //update our current position
	}
	return solution;
}

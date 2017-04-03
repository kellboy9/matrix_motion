#include "dialogue.h"

static SDL_Texture *font = NULL;

//words in 14x16 pt, travels until end of image
void load_font(char *filename, SDL_Renderer *renderer) {
	SDL_Surface *s = SDL_LoadBMP(filename);
	if(!s) {
		printf("Error: Loading %s fail! SDL_Error: %s\n", filename, SDL_GetError());
	}
	SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0xFF, 0x00, 0xFF));

	//this call erases the player texture for some reason.
	font = SDL_CreateTextureFromSurface(renderer, s);
	if(!font) {
		printf("Error: Creating font texture fail! SDL_Error: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(s);
	
}

void draw_message(const char *message, SDL_Renderer *renderer) {
	int line = 0;
	for(int i = 0; message[i]; ++i) {
		//printf("%c: %d\n", message[i], message[i]);
		int charloc = (int)message[i] - 64;
		SDL_Rect src = {charloc*10, 0, 10, 16};
		SDL_Rect dst = {i * 10, line * 16, 10, 16};
		SDL_RenderCopy(renderer, font, &src, &dst);
	}
}

#include <SDL2/SDL.h>
#include "list.h"
#include "entity.h"

lua_State *L; //there can be only one

struct entity {
	char *name; //used for lookup
	SDL_Texture *draw; //graphic to draw (in memory) TODO: read from db, not local
	SDL_Rect pos; //x, y, w, h (dst rect)
	SDL_Rect anim; //animation frame (src rect from draw)
	List *components; //list of components to check for, of type Component
};


typedef struct component Component;
struct component {
	int luaref;
	List *inputs; //list of Event* structs
	Entity *parent;
};

//no global Entities list bc different levels, etc. may have different entity stores
//but there is a global list of components and it's a hard array
static Component **components = NULL; //init size of 2048, but will be realloced if needed
int components_index = 0; 
int components_max = 2048; //keep track of storage space
Component *push_component(int ref, List *inputs, Entity *parent) {
	if(!components) {
		components = malloc(sizeof(Component*) * components_max);
	}
	if(components_index + 1 > components_max) {
		components_max *= 2;
		components = realloc(components, sizeof(Component*) * components_max);
	}

	if(!components) {
		printf("Error: Allocation failed for components array!\n");
		exit(1);
	}

	components[components_index] = malloc(sizeof(Component));
	components[components_index]->luaref = ref;
	components[components_index]->inputs = inputs;
	components[components_index]->parent = parent;
	components_index++;
	return components[components_index-1];
}

static void stackDump (lua_State *L) {
	int i;
	int top = lua_gettop(L);
	printf("Lua: Stack dump:");
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t) {

			case LUA_TSTRING:  /* strings */
				printf("`%s'", lua_tostring(L, i));
				break;

			case LUA_TBOOLEAN:  /* booleans */
				printf(lua_toboolean(L, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:  /* numbers */
				printf("%g", lua_tonumber(L, i));
				break;

			default:  /* other values */
				printf("%s", lua_typename(L, t));
				break;

		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}

void init_entities() {
	L = lua_open();
	luaL_openlibs(L);
	//TODO: register 'locate_entity' function 
}


struct event {
	char *event_name;
	char *input_type;
	char *input_name;
};
List *register_inputs(char *filename) {
	printf("Registering from %s.\n", filename);
	List *inputs = new_list();
	FILE *fp = fopen(filename, "r");
	char line[2048]; //stack hog
	while(fgets(line, 2048, fp)) {
		char *event_name = malloc(sizeof(char)*256); //heap hog
		char *input_type = malloc(sizeof(char)*16);
		char *input_name = malloc(sizeof(char)*16);
		printf("Event: %s", line);
		if(line[0] == '#') //skip comments
			continue;
		sscanf(line, "%s %s %s", event_name, input_type, input_name);
		printf("Event confirmed: %s: %s %s\n", event_name, input_type, input_name);
		Event *new_event = malloc(sizeof(Event));
		new_event->event_name = event_name;
		new_event->input_type = input_type;
		new_event->input_name = input_name;
		printf("Loaded event.\n");
		add_item(inputs, new_event);
		printf("Added to events list.\n");
	}
	return inputs;
}

//TODO: cache these somewhere
Component *load_component(char *name, Entity *parent) {	
	char script_name[256];	 //way big filename but whatever
	if(strlen(name) + 16 > 256) {
		printf("Error: Component name exceeds buffer length!\n");
		return NULL;
	}
	strcpy(script_name, "objects/");
	strcat(script_name, name);
	strcat(script_name, ".lua");
	printf("Reading %s...\n", script_name);
	if(luaL_loadfile(L, script_name) || lua_pcall(L, 0, 0, 0)) {
		printf("Error: Cannot load script file for %s\n.", name);
	}

	printf("Lua: Setting metatable.\n");
	//get the metatable
	lua_getglobal(L, name); //get the component table, pushes it to -1
	if(lua_isnil(L, -1)) {
		printf("Error: %s table is nil!\n", name);
	}
	lua_pushvalue(L, -1); //get the component table again
	lua_setfield(L, -2, "__index"); //component.__index = component; make into metatable
	lua_newtable(L); //-1 is {}, pushes 'called' metatable to -2
	lua_pushvalue(L, -2); //get the metatable to the top
	lua_setmetatable(L, -2); //metatable's copy is subsequently popped
	//-1 is now the new table, but metatable set properly

	printf("Collecting reference to component.\n");
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	List *events = NULL;
	lua_getfield(L, -1, "inputs");
	if(!lua_isnil(L, -1)) {
		printf("Lua: Script has inputs. Registering them...\n");
		char inputs_filename[256];
		strcpy(inputs_filename, "objects/");
		strcat(inputs_filename, name);
		strcat(inputs_filename, ".inputs");
		events = register_inputs(inputs_filename);
	}

	printf("Pushing component...\n");
	Component *comp = push_component(ref, events, parent);//store a reference so that we can keep the stack clean
	lua_pop(L, 2); //pop the metatable, stack returns to 0
	stackDump(L);
	return comp;
}

void set_parent(int luaref, Entity *parent) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, luaref); //grab the table
	lua_newtable(L); //create 'parent' table

	lua_newtable(L); //create 'pos' table
	lua_pushnumber(L, parent->pos.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, parent->pos.y);
	lua_setfield(L, -2, "y");
	lua_pushnumber(L, parent->pos.w);
	lua_setfield(L, -2, "w");
	lua_pushnumber(L, parent->pos.h);
	lua_setfield(L, -2, "h");
	lua_setfield(L, -2, "pos");

	lua_newtable(L); //create 'anim' table
	lua_pushnumber(L, parent->anim.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, parent->anim.y);
	lua_setfield(L, -2, "y");
	lua_pushnumber(L, parent->anim.w);
	lua_setfield(L, -2, "w");
	lua_pushnumber(L, parent->anim.h);
	lua_setfield(L, -2, "h");
	lua_setfield(L, -2, "anim");

	lua_setfield(L, -2, "parent"); //pops new table

	lua_pop(L, 1); //pop the table
}

Entity *new_entity(char *name, SDL_Renderer *renderer, int x, int y) {
	//loda name.ent
	//move through component with fgets
	//load_component for each component, check if loaded or not, if not, then load_component
	//stores in entities list
	printf("Creating new entity in memory...\n");
	Entity *new = malloc(sizeof(Entity));
	new->pos.x = x;
	new->pos.y = y;
	new->components = new_list();

	char entpath[256];
	strcpy(entpath, "objects/");
	strcat(entpath, name);
	strcat(entpath, ".ent");
	FILE *entfile = fopen(entpath, "r");
	if(!entfile) {
		printf("Could not read %s.\n", name);
	}
	char line[2048];

	//get sprite
	printf("Loading sprite...\n");	
	fgets(line, 2048, entfile);
	line[strcspn(line, "\n")] = 0;
	char spritepath[256];	
	strcpy(spritepath, "objects/");
	strcat(spritepath, line);
	SDL_Surface *s = SDL_LoadBMP(spritepath);
	if(!s) {
		printf("Loading %s fail! SDL_Error: %s\n", spritepath, SDL_GetError());
	}
	SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0xFF, 0x00, 0xFF));
	new->pos.w = s->w;
	new->pos.h = s->h;
	new->anim.w = s->w;
	new->anim.h = s->h;
	new->draw = SDL_CreateTextureFromSurface(renderer, s);
	if(!new->draw) {
		printf("Creating entity texture fail! SDL_Error: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(s);

	printf("Begin loading components...\n");
	//now loop through and get components
	while(fgets(line, 2048, entfile)) {
		line[strcspn(line, "\n")] = 0; //remove trailing newline
		char path[256];
		strcpy(path, "objects/");
		strcat(path, line);
		printf("Loading component %s.\n", path);
		Component *comp = load_component(line, new);
		printf("Component loaded. Adding to current entity...\n");
		//inject 'parent' table
		set_parent(comp->luaref, new);
		add_item(new->components, comp);
	}
	printf("Entity component list populated.\n");

	return new;
}

void update_component(Component *component) {
	//printf("Updating id %d.\n", ref);	
	//stackDump(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, component->luaref); //grab the table
	
	//push parent vars
	lua_getfield(L, -1, "parent");

	lua_getfield(L, -1, "pos");
	lua_getfield(L, -1, "x");
	if(lua_isnumber(L, -1)) {
		component->parent->pos.x = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "y");
	if(lua_isnumber(L, -1)) {
		component->parent->pos.y = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "w");
	if(lua_isnumber(L, -1)) {
		component->parent->pos.w = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "h");
	if(lua_isnumber(L, -1)) {
		component->parent->pos.h = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_pop(L, 1); //pop 'pos'

	lua_getfield(L, -1, "anim");
	lua_getfield(L, -1, "x");
	if(lua_isnumber(L, -1)) {
		component->parent->anim.x = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "y");
	if(lua_isnumber(L, -1)) {
		component->parent->anim.y = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "w");
	if(lua_isnumber(L, -1)) {
		component->parent->anim.w = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "h");
	if(lua_isnumber(L, -1)) {
		component->parent->anim.h = lua_tonumber(L, -1);	
	}
	lua_pop(L, 1);
	lua_pop(L, 1); //pop 'anim'

	lua_pop(L, 1); //pop parent

	//call update function
	lua_getfield(L, -1, "update");
	lua_pushvalue(L, -2); //put the needed table at the top of the stack for the 'self' reference
	//State, argc, retc, errfunc
	if(lua_pcall(L, 1, 0, 0) != 0) {
		printf("error: %s\n", lua_tostring(L, -1));
	}
	//lua_pcall pops the 'update function', so no explicit lua_pop required
	lua_pop(L, 1); //pop the table
}

void update_components() {
	for(int i = 0; i < components_index; ++i) {
		update_component(components[i]);
	}
}

const char *get_event_from_input(List *events, const char *input_name) {
	Node *i = NULL;
	while(i = next(events, i)) {
		Event *ev = item(i);
		if(strcmp(ev->input_name, input_name) == 0) {
			return ev->event_name;	
		}
	}
	return "";
}


//t( be run every update cycle in main
void dispatch_events(SDL_Event e) {
	for(int i = 0; i < components_index; ++i) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, components[i]->luaref); //pushes component table to stack
		if(!components[i]->inputs) {
			lua_pop(L, 1); //pop component table
			continue; //NO INPUTS, SO SKIP IT
		}
		lua_getfield(L, -1, "inputs"); //pushes "inputs" table 
		if(!lua_isnil(L, -1)) { //double-checking, highly unnecessary
			const char *ev = get_event_from_input(components[i]->inputs, SDL_GetKeyName(e.key.keysym.sym));
			if(strcmp(ev, "") == 0) {
				lua_pop(L, 2); //pop inputs table + component table
				continue; //component doesn't respond to the current event, so skip it
			}
			if(e.type == SDL_KEYDOWN) {
				lua_getfield(L, -1, ev); //get the appropriate event from inputs table
				if(!lua_isnil(L, -1)) {
					lua_pop(L, 1); //pop the value, if it's not nil we don't need it
					lua_pushboolean(L, 1);
					lua_setfield(L, -2, ev); //set it here
				}
			} else if(e.type == SDL_KEYUP) {
				lua_getfield(L, -1, ev); //get the appropriate event from inputs table
				if(!lua_isnil(L, -1)) {
					lua_pop(L, 1);
					lua_pushboolean(L, 0);
					lua_setfield(L, -2, ev);
				}
			}
		}
		lua_pop(L, 2); //pop the current component + inputs table
	}
}

void draw_entity(Entity *ent, SDL_Renderer *renderer) {
	SDL_RenderCopy(renderer, ent->draw, &(ent->anim), &(ent->pos));
}

void free_entities(List *entities) {
	for(Node *ei = NULL; ei; ei = next(entities, ei)) {
		Entity *e = item(ei);
		for(Node *ci = NULL; ci; ci = next(e->components, ci)) {
			Component *c = item(ci);
			for(Node *ii = NULL; ii; ii = next(c->inputs, ii)) {
				Event *i = item(ii);
				free(i->event_name);
				free(i->input_type);
				free(i->input_name);
				free(i);
			}
			free_list(c->inputs);
			free(c);
		}
		free_list(e->components);
		SDL_DestroyTexture(e->draw);
		free(e);
	}
	free_list(entities);
}

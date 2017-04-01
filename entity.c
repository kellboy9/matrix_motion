#include <SDL2/SDL.h>
#include "list.h"
#include "entity.h"

lua_State *L; //there can be only one

typedef struct component Component;
struct component {
	int luaref;
	List *inputs; //list of Event* structs
};
//no global Entities list bc different levels, etc. may have different entity stores
//but there is a global list of components and it's a hard array
static Component **components = NULL; //init value of 2048, but will be realloced if needed
int components_index = 0; 
int components_max = 2048; //keep track of storage space
Component *push_component(int ref, List *inputs) {
	if(!components) {
		components = malloc(sizeof(Component*) * components_max);
	}
	if(components_index + 1 > components_max) {
		components_max *= 2;
		components = realloc(components, sizeof(Component*) * components_max);
	}

	components[components_index] = malloc(sizeof(Component));
	components[components_index]->luaref = ref;
	components[components_index]->inputs = inputs;
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


//TODO: cache these somewhere
Component *load_component(char *name) {	
	char script_name[256];	 //way big filename but whatever
	strcpy(script_name, "objects/");
	strcat(script_name, name);
	strcat(script_name, ".lua");
	printf("Reading %s...\n", script_name);
	if(luaL_loadfile(L, script_name) || lua_pcall(L, 0, 0, 0)) {
		printf("Cannot load script file for %s\n.", name);
	}

	printf("Lua: setting metatable.\n");
	//get the metatable
	lua_getglobal(L, name); //get the component table, pushes it to -1
	if(lua_isnil(L, -1)) {
		printf("%s table is nil!\n", name);
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
		printf("Script has inputs. Registering them...\n");
		char inputs_filename[2048];
		strcpy(inputs_filename, "objects/");
		strcat(inputs_filename, name);
		strcat(inputs_filename, ".inputs");
		events = register_inputs(inputs_filename);
	}

	printf("Pushing component...\n");
	Component *comp = push_component(ref, events);//store a reference so that we can keep the stack clean
	lua_pop(L, 2); //pop the metatable, stack returns to 0
	stackDump(L);
	return comp;
}

Entity *new_entity(char *name, SDL_Renderer *renderer) {
	//loda name.ent
	//move through component with fgets
	//load_component for each component, check if loaded or not, if not, then load_component
	//stores in entities list
	printf("Creating new entity in memory...\n");
	Entity *new = malloc(sizeof(Entity));
	new->pos.x = 0;
	new->pos.y = 0;
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
		Component *comp = load_component(line);
		printf("Component loaded. Adding to current entity...\n");
		add_item(new->components, comp);
	}
	printf("Entity component list populated.\n");

	return new;
}

Entity *new_entity_pos(char *name, SDL_Renderer *renderer, int x, int y) {
	Entity *new = new_entity(name, renderer);
	new->pos.x = x;
	new->pos.y = y;
	return new;
}


void update_component(int ref) {
	//printf("Updating id %d.\n", ref);	
	//stackDump(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref); //grab the table
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
		update_component(components[i]->luaref);
	}
}

//EVENT SYSTEM
struct event {
	char *event_name;
	char *input_type;
	char *input_name;
};

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
					lua_pushboolean(L, 1);
					lua_setfield(L, -2, ev);
				}
			}
		}
		lua_pop(L, 2); //pop the current component + inputs table
	}
}


//TODO: FREE FUCNTION TO CLEAN UP THIS MESS OF ALLOCS

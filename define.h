_Pragma("once")
#include "header.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_MIDDLE_X (SCREEN_WIDTH / 2)
#define SCREEN_MIDDLE_Y (SCREEN_HEIGHT / 2)

#define TILE_SIZE 24

#define padding_top 24
#define padding_bottom 16
#define padding_left 16
#define padding_right 16

#define start_x padding_left
#define start_y (40 + padding_top)
#define end_x (SCREEN_WIDTH - (padding_left + padding_right))
#define end_y (SCREEN_HEIGHT - (32 + 40 + padding_top + padding_bottom))

#define MAX_SURFACES 30
#define MAX_TEXTURES 5
#define MAX_OBJECTS 500

    typedef struct {
    SDL_Surface *sprite[9];
} SPRITESHEET_T;
typedef struct {
    SDL_Surface *surfaces[MAX_SURFACES];
    SDL_Texture *textures[MAX_TEXTURES];
    SDL_Window *window;
    SDL_Renderer *renderer;
    int surface_index = 0;
    int texture_index = 0;
} SDL_OBJECTS_T;

enum Direction { RIGHT, LEFT, UP, DOWN };
enum ObjectType { BORDER, LADDER, LADDER_TOP, PLATFORM, ENEMY, NOTHING };
enum PLATFORM_TYPE { PLATFORM_MEDIUM, PLATFORM_SHORT, PLATFORM_LONG };
enum LADDER_TYPE { LADDER_SHORT, LADDER_MEDIUM, LADDER_LONG };

#define JUMP_HEIGHT 1
typedef struct {
    int curent_sprite = 0;
    int anim_cycle = 0;
    double x = 0;
    double y = 0;
} START_VALUES_OBJECT_T;
typedef struct {
    int ladder_possible = 0;
    int ladder_state = 0;
    int ladder_top = 0;
    int dead_state = 0;
    int jump_state = 0;
    int moving = 0;
    int falling = 1;
} START_VALUES_PLAYER_T;
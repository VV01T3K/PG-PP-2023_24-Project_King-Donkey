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
#define MAX_OBJECTS 200

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
enum ObjectType { BORDER, LADDER, LADDER_TOP, PLATFORM, BARREL, WIN, NOTHING };
enum PLATFORM_TYPE { PLATFORM_SHORT, PLATFORM_MEDIUM, PLATFORM_LONG };
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

typedef struct {
    int playing = 1;
    int win = 0;
    int lose = 0;
    int score = 0;
    int lives = 3;
    int level = 1;
    int level_max = 3;
} GAME_T;

// BUILD_LEVEL

#define __BUILDLEVEL_PREPARE__      \
    player.reset();                 \
    for (int i = 0; i < max; i++) { \
        objectList[i]->destroy();   \
    }                               \
    struct {                        \
        int PLATFORM_SHORT_ = 0;    \
        int PLATFORM_MEDIUM_ = 0;   \
        int PLATFORM_LONG_ = 0;     \
        int LADDER_SHORT_ = 0;      \
        int LADDER_MEDIUM_ = 0;     \
        int LADDER_LONG_ = 0;       \
        int LADDER_TOP_ = 0;        \
        int BARREL_ = 0;            \
    } index;

enum BUILDLEVEL {
    PLATFORM_SHORT__ = 10,
    PLATFORM_MEDIUM__ = 0,
    PLATFORM_LONG__ = 20,
    LADDER_SHORT__ = 40,
    LADDER_MEDIUM__ = 30,
    LADDER_LONG__ = 50,
    LADDER_TOP__ = 60,
    BARREL__ = 70,
    WIN_ = 80
};

#define NEXT(obj) obj##__ + index.obj##_++
#define LAST(obj) obj##__ + index.obj##_ - 1

#define PLACE(obj, x, y) objectList[obj##__ + index.obj##_++]->place(x, y)

#define MAX_BARRELS 10
#define BARREL_SPEED 0.5

#define PATH_LENGHT 20

typedef struct {
    int cur_x = 0;
    int cur_y = 0;
    int x[PATH_LENGHT] = {0};
    int y[PATH_LENGHT] = {0};
} PATH_T;

#define SET_PATH(n, PATH_X, PATH_Y)           \
    for (int i = 0; i < PATH_LENGHT; i++) {   \
        objectList[n]->path.x[i] = PATH_X[i]; \
        objectList[n]->path.y[i] = PATH_Y[i]; \
    }
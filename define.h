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
#define MAX_BARRELS 10

#define BARREL_SPEED 1
#define PATH_LENGHT 20
#define MONKE_DANCE_TIME 4

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

enum Direction { NONE, RIGHT, LEFT, UP, DOWN };
enum ObjectType {
    BORDER,
    LADDER,
    LADDER_TOP,
    PLATFORM,
    BARREL,
    WIN,
    MONKE,
    NOTHING
};
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

enum BUILDLEVEL {
    PLATFORM_SHORT__ = 10,
    PLATFORM_MEDIUM__ = 0,
    PLATFORM_LONG__ = 20,
    LADDER_SHORT__ = 40,
    LADDER_MEDIUM__ = 30,
    LADDER_LONG__ = 50,
    LADDER_TOP__ = 60,
    WIN_ = 70,
    MONKE_ = 71,
};

#define __BUILDLEVEL_PREPARE__             \
    int i = 0;                             \
    objectList[MONKE_]->curent_sprite = 0; \
    objectList[MONKE_]->ready_throw = 0;   \
    objectList[MONKE_]->monke_dance = 0;   \
    player.reset();                        \
    for (i = 0; i < max; i++) {            \
        objectList[i]->destroy();          \
    }                                      \
    for (i = 0; i < barrelMax; i++) {      \
        barrelList[i]->destroy();          \
        barrelList[i]->reset();            \
    }                                      \
    struct {                               \
        int PLATFORM_SHORT_ = 0;           \
        int PLATFORM_MEDIUM_ = 0;          \
        int PLATFORM_LONG_ = 0;            \
        int LADDER_SHORT_ = 0;             \
        int LADDER_MEDIUM_ = 0;            \
        int LADDER_LONG_ = 0;              \
        int LADDER_TOP_ = 0;               \
        int BARREL_ = 0;                   \
    } index;

#define NEXT(obj) obj##__ + index.obj##_++
#define LAST(obj) obj##__ + index.obj##_ - 1

#define PLACE(obj, x, y) objectList[obj##__ + index.obj##_++]->place(x, y)

#define PLACE_BARREL(x, y, PATH_XX, PATH_YY, DELAY)        \
    barrelList[index.BARREL_++]->place(x, y);              \
    barrelList[index.BARREL_ - 1]->delay = DELAY;          \
    i = 0;                                                 \
    while (PATH_XX[i] != 100)                              \
        barrelList[index.BARREL_ - 1]->path_x[i] =         \
            SCREEN_WIDTH / 2 + (PATH_XX[i++] * TILE_SIZE); \
    i = 0;                                                 \
    while (PATH_YY[i] != 100)                              \
        barrelList[index.BARREL_ - 1]->path_y[i] =         \
            SCREEN_HEIGHT / 2 - (PATH_YY[i++] * TILE_SIZE) + 1.5;\

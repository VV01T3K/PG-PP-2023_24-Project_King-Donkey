_Pragma("once")
#include "header.h"

#define SCREEN_WIDTH 640   // The width of the game screen in pixels
#define SCREEN_HEIGHT 480  // The height of the game screen in pixels

#define TILE_SIZE 24  // The size of a tile in the game in pixels

#define padding_top 24  // The padding at the top of the game screen in pixels

// The padding at the bottom of the game screen in pixels
#define padding_bottom 16
// The padding on the left of the game screen in pixels
#define padding_left 16
// The padding on the right of the game screen in pixels
#define padding_right 16

#define start_x padding_left        // The starting x-coordinate for the game
#define start_y (40 + padding_top)  // The starting y-coordinate for the game

// The ending x-coordinate for the game
#define end_x (SCREEN_WIDTH - (padding_left + padding_right))
// The ending y-coordinate for the game
#define end_y (SCREEN_HEIGHT - (32 + 40 + padding_top + padding_bottom))

#define MAX_SURFACES 30      // The maximum number of surfaces in the game
#define MAX_TEXTURES 5       // The maximum number of textures in the game
#define MAX_OBJECTS 200      // The maximum number of objects in the game
#define MAX_BARRELS 10       // The maximum number of barrels in the game
#define MAX_POPUPS 10        // The maximum number of popups in the game
#define MAX_POPUP_LENGHT 20  // The maximum length of a popup in the game

// The height a barrel can jump over in the game
#define BARREL_JUMP_OVER_HEIGHT 1
#define POPUP_SPEED 1       // The speed of popups in the game
#define BARREL_SPEED 1      // The speed of barrels in the game
#define PATH_LENGHT 20      // The length of the path in the game
#define MONKE_DANCE_TIME 4  // The time for the monkey to dance in the game
#define JUMP_HEIGHT 1       // The height of a jump in the game
#define ANIMATION_SPEED 1   // The speed of animation in the game

// scores
// The score for jumping over a barrel in the game
#define OVER_BARREL_SCORE 300
#define WIN_LEVEL_SCORE 1000      // The score for winning a level in the game
#define COIN_COLLECTED_SCORE 800  // The score for collecting a coin in the game
#define DEATH_PENALTY 500         // The penalty for dying in the game

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
    COIN,
    TEXT,
    NOTHING
};

enum PLATFORM_TYPE { PLATFORM_SHORT, PLATFORM_MEDIUM, PLATFORM_LONG };
enum LADDER_TYPE { LADDER_SHORT, LADDER_MEDIUM, LADDER_LONG };

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
    double levelTime = 0;
} GAME_T;

// BUILD_LEVEL

// Its to keep track of indexes for objects
enum BUILDLEVEL {
    PLATFORM_SHORT__ = 10,
    PLATFORM_MEDIUM__ = 0,
    PLATFORM_LONG__ = 20,
    LADDER_SHORT__ = 40,
    LADDER_MEDIUM__ = 30,
    LADDER_LONG__ = 50,
    LADDER_TOP__ = 60,
    COIN__ = 70,
    WIN_ = 80,
    MONKE_ = 81,
};

// The code block initializes several variables
// and resets the state of various
// objects and variables.
#define __BUILDLEVEL_PREPARE__             \
    int i = 0;                             \
    objectList[WIN_]->curent_sprite = 0;   \
    objectList[MONKE_]->curent_sprite = 0; \
    objectList[MONKE_]->ready_throw = 0;   \
    objectList[MONKE_]->monke_dance = 0;   \
    player.reset();                        \
    for (i = 0; i < max; i++) {            \
        objectList[i]->destroy();          \
    }                                      \
    for (i = 0; i < barrelMax; i++) {      \
        barrelList[i]->reset();            \
        barrelList[i]->destroy();          \
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
        int COIN_ = 0;                     \
        int TEXT_ = 0;                     \
    } index;

#define NEXT(obj) obj##__ + index.obj##_++
#define LAST(obj) obj##__ + index.obj##_ - 1

#define PLACE(obj, x, y) objectList[obj##__ + index.obj##_++]->place(x, y)

// The `#define PLACE_BARREL(x, y, PATH_XX, PATH_YY, DELAY)` macro is used to
// place a barrel object in the game. It takes in the x and y coordinates of the
// barrel's initial position, an array `PATH_XX` representing the x-coordinates
// of the barrel's path, an array `PATH_YY` representing the y-coordinates of
// the barrel's path, and a `DELAY` value representing the delay before the
// barrel starts moving.
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

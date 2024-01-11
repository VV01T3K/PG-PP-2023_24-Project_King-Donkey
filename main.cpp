#include "header.h"

// narysowanie napisu txt na powierzchni screen, zaczynaj�c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj�ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while (*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8;
        text++;
    };
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt �rodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
    SDL_Rect dest;
    dest.x = x - sprite->w / 2;
    dest.y = y - sprite->h / 2;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = color;
};

// rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1)
// b�d� poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0)
// line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy,
              Uint32 color) {
    for (int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
    };
};

// rysowanie prostok�ta o d�ugo�ci bok�w l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for (i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

/**
 * The function frees SDL resources by freeing surfaces, textures, window,
 * renderer, and quitting SDL.
 *
 * @param sdl_objects sdl_objects is a pointer to a struct that contains various
 * SDL objects such as surfaces, textures, window, and renderer.
 */
void free_sdl_resources(SDL_OBJECTS_T *sdl_objects) {
    for (int i = 0; i < sdl_objects->surface_index; i++) {
        SDL_FreeSurface(sdl_objects->surfaces[i]);
    }
    for (int i = 0; i < sdl_objects->texture_index; i++) {
        SDL_DestroyTexture(sdl_objects->textures[i]);
    }
    SDL_DestroyWindow(sdl_objects->window);
    SDL_DestroyRenderer(sdl_objects->renderer);
    SDL_Quit();
}

/**
 * The function loads an image from a specified path into an SDL surface and
 * stores the surface in an array of surfaces.
 *
 * @param surface A pointer to a pointer to an SDL_Surface. This is used to
 * store the loaded image surface.
 * @param image_path A string representing the name of the image file to be
 * loaded.
 * @param sdl_obj sdl_obj is a pointer to a structure of type SDL_OBJECTS_T.
 * This structure contains various SDL objects and resources that are used in
 * the program.
 *
 * @return an integer value. If the image loading is successful, it returns 0.
 * If there is an error in loading the image, it returns 1.
 */
int load_image_into_surface(SDL_Surface **surface, const char *image_path,
                            SDL_OBJECTS_T *sdl_obj) {
    char imagePath[256];
    sprintf(imagePath, "./Images/%s.bmp", image_path);
    *surface = SDL_LoadBMP(imagePath);
    sdl_obj->surfaces[sdl_obj->surface_index++] = *surface;

    if (*surface == NULL) {
        printf("SDL_LoadBMP(%s) error: %s\n", image_path, SDL_GetError());
        free_sdl_resources(sdl_obj);
        return 1;
    }

    return 0;
}

class OBJECT {
   public:
    SPRITESHEET_T *sheet;
    int monke_dance = 0;
    int ready_throw = 0;
    double frameCounter = 0;
    int curent_sprite = 0;
    int anim_cycle = 0;
    int type;
    double *delta;
    double x = 0;
    double y = 0;
    START_VALUES_OBJECT_T start_values;
    Direction direction = RIGHT;

    OBJECT(int type, int sprite_type, double *delta, SPRITESHEET_T *sheet)
        : type(type), sheet(sheet), delta(delta), curent_sprite(sprite_type) {
        if (type == LADDER_TOP) curent_sprite = 3;

        start_values.curent_sprite = curent_sprite;
        start_values.anim_cycle = anim_cycle;
    }

    void draw(SDL_Surface *screen);
    double getBORDER(Direction side);
    void reset();
    void place(float x, float y);
    void destroy() { x = y = 0; };
    void simple_animation(float speed, int start, int end, int cycle);
};

/**
 * The function performs a simple animation by updating the current sprite based
 * on the given speed, start and end values, and cycle flag.
 *
 * @param speed The speed parameter determines how fast the animation should
 * play. It represents the number of frames per second (fps) at which the
 * animation should be played. For example, if speed is set to 30, the animation
 * will play at 30 frames per second.
 * @param start The start parameter represents the index of the first sprite in
 * the animation sequence.
 * @param end The "end" parameter in the given code represents the index of the
 * last sprite in the animation sequence.
 * @param cycle The "cycle" parameter determines whether the animation should
 * loop or not. If "cycle" is set to 1, the animation will loop from the "start"
 * sprite to the "end" sprite and then start again from the "start" sprite. If
 * "cycle" is set to 0
 *
 * @return In this code, the function is returning nothing (void).
 */
void OBJECT::simple_animation(float speed, int start, int end, int cycle) {
    if (type == MONKE) {
        if (monke_dance == 0) {
            curent_sprite = 0;
            ready_throw = 1;
            return;
        };
    }

    if (frameCounter >= 60 / speed) {
        frameCounter = 0;
        if (type == MONKE) monke_dance -= 1;
        if (cycle) {
            curent_sprite++;
            if (curent_sprite > end) curent_sprite = start;

        } else {
            curent_sprite--;
            if (curent_sprite < start) curent_sprite = end;
        }
    }
}

/**
 * The function "place" is used to set the x and y coordinates of an object on
 * the screen based on given parameters, with some additional adjustments for
 * specific object types.
 *
 * @param x The x parameter represents the horizontal position of the object,
 * relative to the center of the screen. It is multiplied by the size of each
 * tile (TILE_SIZE) and added to half of the screen width (SCREEN_WIDTH / 2) to
 * calculate the final x position of the object.
 * @param y The "y" parameter in the "place" function represents the vertical
 * position of the object on the screen. It is used to calculate the final
 * y-coordinate of the object's position based on the screen height and the size
 * of each tile.
 */
void OBJECT::place(float x, float y) {
    this->x = SCREEN_WIDTH / 2 + (x * TILE_SIZE);
    this->y = SCREEN_HEIGHT / 2 - (y * TILE_SIZE);
    if (type == LADDER_TOP)
        this->y -= TILE_SIZE / 2;
    else if (type == LADDER)
        this->y += (sheet->sprite[curent_sprite]->h - TILE_SIZE) / 2;
    start_values.x = this->x;
    start_values.y = this->y;
}

/**
 * The reset function sets the object's position, current sprite, and animation
 * cycle to their initial values.
 */
void OBJECT::reset() {
    x = start_values.x;
    y = start_values.y;
    curent_sprite = start_values.curent_sprite;
    anim_cycle = start_values.anim_cycle;
}

/**
 * The function "getBORDER" returns the border position of an object in a
 * specified direction.
 *
 * @param side The "side" parameter is of type "Direction" and represents the
 * side of the object for which we want to get the border value. The possible
 * values for "side" are RIGHT, LEFT, UP, and DOWN.
 *
 * @return a double value, which represents the border position of the object in
 * the specified direction.
 */
double OBJECT::getBORDER(Direction side) {
    switch (side) {
        case RIGHT:
            return x + sheet->sprite[curent_sprite]->w / 2;
        case LEFT:
            return x - sheet->sprite[curent_sprite]->w / 2;
        case UP:
            return y - sheet->sprite[curent_sprite]->h / 2;
        case DOWN:
            return y + sheet->sprite[curent_sprite]->h / 2;
        default:
            return 0;
    }
}

/**
 * The draw function checks if the object's position is at (0,0), and if not, it
 * draws the object's sprite on the screen and updates the frame counter.
 *
 * @param screen The "screen" parameter is a pointer to an SDL_Surface object,
 * which represents the screen or window where the object will be drawn.
 *
 * @return If the conditions `x == 0` and `y == 0` are true, then nothing is
 * being returned. The `return` statement will exit the function and no further
 * code will be executed.
 */
void OBJECT::draw(SDL_Surface *screen) {
    if (x == 0 && y == 0) return;
    DrawSurface(screen, sheet->sprite[curent_sprite], x, y);
    frameCounter += *delta * 200;
    if (frameCounter > 1000) frameCounter = 0;
}

/**
 * The function determines if two objects intersect by comparing their borders.
 *
 * @param object1 The first object being checked for intersection.
 * @param object2 The above code defines a function named "intersects" that
 * takes two objects as parameters, object1 and object2. The function checks if
 * object1 intersects with object2 by comparing their borders in each direction
 * (UP, DOWN, LEFT, RIGHT). If there is an intersection, the function returns
 *
 * @return 1 if the two objects intersect, and 0 if they do not intersect.
 */
int intersects(OBJECT &object1, OBJECT &object2) {
    if (object1.getBORDER(DOWN) > object2.getBORDER(UP) &&
        object1.getBORDER(LEFT) < object2.getBORDER(RIGHT) &&
        object1.getBORDER(RIGHT) > object2.getBORDER(LEFT) &&
        object1.getBORDER(UP) < object2.getBORDER(DOWN))
        return 1;
    return 0;
}

class Barrel : public OBJECT {
   public:
    Barrel(double *delta, SPRITESHEET_T *sheet, OBJECT *monke)
        : OBJECT(BARREL, 0, delta, sheet) {
        this->monke = monke;
        this->direction = RIGHT;
    }
    OBJECT *monke;
    int jumped_over = 0;
    void nextFrame(SDL_Surface *screen);
    int start = 0, end = 0;
    double path_x[PATH_LENGHT] = {100};
    double path_y[PATH_LENGHT] = {100};
    int curr_dest = 0;
    int falling = 0;
    double delay = 0;
    int spawned = 0;
    Direction last_direction = direction;
    void reset();
    /**
     * The function "destroy" sets the variables x and y to 0, and initializes
     * the arrays path_x and path_y with the value 100 for each element.
     */
    void destroy() {
        x = y = 0;
        for (int i = 0; i < PATH_LENGHT; i++) {
            path_x[i] = 100;
            path_y[i] = 100;
        }
    };
};

/**
 * The function `nextFrame` updates the position and animation of a barrel
 * object in a game.
 *
 * @param screen The "screen" parameter is a pointer to an SDL_Surface object.
 * It represents the screen surface where the barrel will be drawn.
 *
 * @return The function does not have a return type, so it does not return
 * anything.
 */
void Barrel::nextFrame(SDL_Surface *screen) {
    if (x == 0 && y == 0) return;
    if (delay > 0) {
        delay -= *delta;
        return;
    }

    if (!ready_throw && !spawned) {
        spawned = 1;
        delay = 1;
        monke->monke_dance = 4;
        return;
    }

    if (!spawned) return;

    if (path_x[curr_dest] == 100 || path_y[curr_dest] == 100) {
        spawned = 0;
        this->reset();
    }
    double old_x = x;
    double old_y = y;

    direction = DOWN;
    if (falling && y < path_y[curr_dest]) {
        y += *delta * 200 * BARREL_SPEED;
    }
    if (!falling) {
        if (x < path_x[curr_dest])
            direction = RIGHT;
        else if (x > path_x[curr_dest])
            direction = LEFT;
    }

    anim_cycle = 0;
    switch (direction) {
        case RIGHT:
            x += *delta * 100 * BARREL_SPEED;
            start = 0;
            end = 3;
            anim_cycle = 1;
            if (x > path_x[curr_dest]) falling = 1;
            break;
        case LEFT:
            x -= *delta * 100 * BARREL_SPEED;
            start = 0;
            end = 3;
            anim_cycle = 0;
            if (x < path_x[curr_dest]) falling = 1;
            break;
        case DOWN:
            start = 4;
            end = 5;
            break;
        default:
            break;
    }

    if (old_x == x && old_y == y) {
        curr_dest++;
        falling = 0;
        direction = DOWN;
        start = 4;
        end = 5;
    }
    if (last_direction != direction) frameCounter = 500;
    last_direction = direction;
    OBJECT::simple_animation(2, start, end, anim_cycle);
    OBJECT::draw(screen);
}

/**
 * The function "reset" resets various variables of a Barrel object.
 */
void Barrel::reset() {
    OBJECT::reset();
    falling = 0;
    delay = 0;
    curr_dest = 0;
    last_direction = direction;
    spawned = 0;
    jumped_over = 0;
}

class TextPopup : public OBJECT {
   public:
    SDL_Surface *charset;
    TextPopup(double *delta, SDL_Surface *charset)
        : OBJECT(NOTHING, 0, delta, 0) {
        this->charset = charset;
    }
    char *text;
    void nextFrame(SDL_Surface *screen);
    /**
     * The function "destroy" sets the values of orginal_y, x, and y to 0.
     */
    void destroy() {
        orginal_y = 0;
        x = y = 0;
    };
    void draw(SDL_Surface *screen);
    int orginal_y = 0;
    /**
     * The function "place" takes in three parameters (x, y, and text) and
     * assigns them to the corresponding member variables (x, y, and text) of
     * the object.
     *
     * @param x The x-coordinate of the position where the text will be placed.
     * @param y The y parameter is a float value representing the vertical
     * position where the text will be placed.
     * @param text The "text" parameter is a pointer to a character array, which
     * represents the text that you want to place at the specified coordinates
     * (x, y).
     */
    void place(float x, float y, char *text) {
        orginal_y = y;
        this->text = text;
        this->x = x;
        this->y = y;
    }
    int malloced = 0;
};

/**
 * The function moves a text popup up the screen and destroys it when it reaches
 * a certain position.
 *
 * @param screen The "screen" parameter is a pointer to an SDL_Surface object,
 * which represents the screen or window where the text popup will be drawn.
 *
 * @return In this code snippet, the function is returning nothing (void).
 */
void TextPopup::nextFrame(SDL_Surface *screen) {
    if (x == 0 && y == 0) return;
    y -= *delta * 30 * POPUP_SPEED;
    if (y < orginal_y - 20) {
        if (malloced) free(text);
        destroy();
        return;
    }
    draw(screen);
}

/**
 * The function draws a text popup on an SDL surface at the specified
 * coordinates.
 *
 * @param screen The screen parameter is a pointer to an SDL_Surface object,
 * which represents the screen or window where the text will be drawn.
 *
 * @return If the conditions `x == 0` and `y == 0` are true, then nothing is
 * being returned. The `return` statement is used to exit the function early and
 * return control to the calling function.
 */
void TextPopup::draw(SDL_Surface *screen) {
    if (x == 0 && y == 0) return;
    DrawString(screen, x, y, text, charset);
}

class Player : public OBJECT {
   private:
    OBJECT **objectList;
    Barrel **barrelList;
    TextPopup **popupList;
    int popupListIndex = 0;
    int barrelListMaxIndex;
    int objectListSize;
    double speed = 1.5;
    double max_gravity = 2;
    double gravity;
    double gravity_delta = 0;
    double delta_x = 0;
    double delta_y = 0;
    START_VALUES_PLAYER_T starting_values;
    GAME_T *GAME;

   public:
    int ladder_possible = 0;
    int ladder_state = 0;
    int ladder_top = 0;
    int dead_state = 0;
    int jump_state = 0;
    int moving = 0;
    int falling = 1;
    Player(double *delta, SPRITESHEET_T *sheet, OBJECT **objectList,
           int objectListSize, GAME_T *GAME, Barrel **barrelList,
           int barrelListMaxIndex, TextPopup **popupList)
        : OBJECT(0, 0, delta, sheet),
          objectList(objectList),
          objectListSize(objectListSize),
          GAME(GAME),
          barrelList(barrelList),
          popupList(popupList),
          barrelListMaxIndex(barrelListMaxIndex) {
        this->gravity = this->max_gravity;

        starting_values.ladder_possible = ladder_possible;
        starting_values.ladder_state = ladder_state;
        starting_values.ladder_top = ladder_top;
        starting_values.dead_state = dead_state;
        starting_values.jump_state = jump_state;
        starting_values.moving = moving;
        starting_values.falling = falling;
    }

    void move(Direction direction);
    void animate();
    void collision();
    void nextFrame(SDL_Surface *screen);
    void jump();
    void reset();
    void popup(char *text, int integer);
};

/**
 * The function `popup` displays a popup message with the given text and integer
 * value.
 *
 * @param text A pointer to a character array that represents the text to be
 * displayed in the popup.
 * @param integer The "integer" parameter is an integer value that is used to
 * determine whether or not to display a popup with a numerical value. If the
 * value of "integer" is non-zero, a popup with the value of "integer" will be
 * displayed.
 */
void Player::popup(char *text, int integer) {
    if (integer) {
        text = (char *)malloc(sizeof(char) * 10);
        sprintf(text, "%d", integer);
        popupList[popupListIndex]->malloced = 1;
    }
    popupList[popupListIndex++]->place(getBORDER(LEFT), getBORDER(UP) - 10,
                                       text);
    if (popupListIndex >= MAX_POPUPS) popupListIndex = 0;
}

/**
 * The function resets the player's attributes to their starting values.
 */
void Player::reset() {
    OBJECT::reset();
    ladder_possible = starting_values.ladder_possible;
    ladder_state = starting_values.ladder_state;
    ladder_top = starting_values.ladder_top;
    dead_state = starting_values.dead_state;
    jump_state = starting_values.jump_state;
    moving = starting_values.moving;
    falling = starting_values.falling;
}

/**
 * The jump function allows the player to jump if certain conditions are met.
 *
 * @return If any of the following conditions are true: jump_state is true,
 * ladder_state is true, dead_state is true, falling is true, or GAME->playing
 * is false, then the function will return and nothing will be executed.
 */
void Player::jump() {
    if (jump_state || ladder_state || dead_state || falling || !GAME->playing)
        return;
    jump_state = 1;
    gravity = max_gravity * -(JUMP_HEIGHT)*speed;
}

/**
 * The function updates the player's position and applies gravity if the game is
 * currently playing.
 *
 * @param screen The "screen" parameter is a pointer to an SDL_Surface object,
 * which represents the screen or window where the game is being displayed.
 *
 * @return In this code, the function is returning void, which means it is not
 * returning any value.
 */
void Player::nextFrame(SDL_Surface *screen) {
    if (GAME->playing == 0) {
        draw(screen);
        return;
    }

    if (gravity < max_gravity) gravity += *delta * max_gravity * 4;
    if (gravity > max_gravity) gravity = max_gravity;

    double gravity_distance = gravity * *delta * 100 * speed;

    gravity_delta = 0;
    if (ladder_state == 0) {
        gravity_delta = gravity_distance;
        y += gravity_distance;
    }
    ladder_possible = 0;
    ladder_top = 0;

    collision();
    draw(screen);

    delta_x = 0;
    delta_y = 0;
}

/**
 * The function checks for collisions between the player and various objects
 * in the game and handles the corresponding actions.
 */
void Player::collision() {
    int horizontalSTOP = 0, verticalSTOP = 0;
    int UNALIVE = 0;
    falling = 1;
    int zatrzymantko = 0;
    ladder_top = 0;
    // check for collision with screen borders
    if (getBORDER(LEFT) < start_x) horizontalSTOP = 1;
    if (getBORDER(RIGHT) > end_x + start_x) horizontalSTOP = 1;
    if (getBORDER(UP) < start_y + 2) {
        verticalSTOP = 1;
        if (gravity < 0) gravity = 0;
        zatrzymantko = 1;
    }
    if (getBORDER(DOWN) > end_y + start_y) {
        verticalSTOP = 1;
        y -= gravity_delta;
        jump_state = 0;
        falling = 0;
        UNALIVE = 1;
    }
    // check for collision with objects
    for (int i = 0; i < objectListSize; i++) {
        if (objectList[i] == NULL) continue;
        if (objectList[i]->type == NOTHING) continue;
        if (objectList[i]->x == 0 && objectList[i]->y == 0) continue;
        if (objectList[i]->type == PLATFORM) {
            if (intersects(*this, *objectList[i])) {
                y -= gravity_delta;
                zatrzymantko = 1;
                if (getBORDER(DOWN) < objectList[i]->getBORDER(UP)) {
                    jump_state = 0;
                    falling = 0;
                }
                if (ladder_state == 0) verticalSTOP = 1;
                if (getBORDER(UP) > objectList[i]->getBORDER(UP)) {
                    jump_state = 1;
                    if (gravity < 0) gravity = 0;
                }
            }
            if (intersects(*this, *objectList[i])) {
                horizontalSTOP = 1;
                y += gravity_delta;
            }
        }
        if (objectList[i]->type == LADDER) {
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                x < objectList[i]->getBORDER(RIGHT) &&
                x > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
                ladder_possible = 1;
            }
        }
        if (objectList[i]->type == LADDER_TOP) {
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                x < objectList[i]->getBORDER(RIGHT) &&
                x > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
                ladder_top = 1;
                zatrzymantko = 0;
            }
        }
        if (objectList[i]->type == WIN) {
            if (intersects(*this, *objectList[i])) {
                if (GAME->playing) {
                    GAME->win += 1;
                    GAME->score += WIN_LEVEL_SCORE;
                    popup("", WIN_LEVEL_SCORE);
                    GAME->playing = 0;
                }
            }
        }
        if (objectList[i]->type == MONKE) {
            if (intersects(*this, *objectList[i])) {
                UNALIVE = 1;
            }
        }
        if (objectList[i]->type == COIN) {
            if (intersects(*this, *objectList[i])) {
                objectList[i]->destroy();
                GAME->score += COIN_COLLECTED_SCORE;
                popup("", COIN_COLLECTED_SCORE);
            }
        }
    }
    for (int i = 0; i < barrelListMaxIndex; i++) {
        if (barrelList[i] == NULL) continue;
        if (barrelList[i]->type == NOTHING) continue;
        if (barrelList[i]->x == 0 && barrelList[i]->y == 0) continue;
        if (getBORDER(DOWN) + (TILE_SIZE / 3) > barrelList[i]->getBORDER(UP) &&
            getBORDER(LEFT) + (TILE_SIZE / 3) <
                barrelList[i]->getBORDER(RIGHT) &&
            getBORDER(RIGHT) - (TILE_SIZE / 3) >
                barrelList[i]->getBORDER(LEFT) &&
            getBORDER(UP) - (TILE_SIZE / 3) < barrelList[i]->getBORDER(DOWN)) {
            UNALIVE = 1;
        }
        if (getBORDER(LEFT) < barrelList[i]->getBORDER(RIGHT) &&
            getBORDER(RIGHT) > barrelList[i]->getBORDER(LEFT) &&
            getBORDER(DOWN) < barrelList[i]->getBORDER(UP) &&
            getBORDER(UP) < barrelList[i]->getBORDER(DOWN)) {
            if (getBORDER(DOWN) <
                barrelList[i]->getBORDER(UP) + JUMP_BARREL_HEIGHT) {
                if (barrelList[i]->jumped_over == 0 && dead_state == 0 &&
                    jump_state == 1 && ladder_state == 0) {
                    GAME->score += OVER_BARREL_SCORE;
                    barrelList[i]->jumped_over = 1;
                    popup("", OVER_BARREL_SCORE);
                }
            }
        }
    }

    if (zatrzymantko && delta_y > 0) y -= delta_y;

    // EVENTS
    if (UNALIVE) {
        if (dead_state == 0) popup("You Died", 0);
        dead_state = 1;
    }
    if (horizontalSTOP) x -= delta_x;
    if (verticalSTOP) y -= delta_y;
}

/**
 * The function "animate" determines the current sprite to display based on the
 * player's state and movement.
 *
 * @return The function does not have a return type, so it does not return
 * anything.
 */
void Player::animate() {
    if (dead_state) {
        curent_sprite = 8;
        return;
    }

    if (GAME->playing == 0) {
        if (frameCounter >= 30) {
            frameCounter = 0;
            curent_sprite++;
            if (curent_sprite > 5) curent_sprite = 0;
        }
        return;
    }

    if (ladder_state) {
        if (curent_sprite != 6 && curent_sprite != 7) curent_sprite = 6;
        if (frameCounter >= 60 && moving) {
            frameCounter = 0;
            curent_sprite = curent_sprite == 7 ? 6 : 7;
        }

        return;
    }
    if (jump_state) {
        curent_sprite = 1;
        if (direction == RIGHT) curent_sprite = 4;

        return;
    }

    if (moving == 0) {
        curent_sprite = 0;
        if (direction == RIGHT) curent_sprite = 3;

        return;
    }
    if (frameCounter >= 30) {
        frameCounter = 0;
        if (direction == LEFT)
            curent_sprite = curent_sprite == 2 ? 0 : 2;
        else
            curent_sprite = curent_sprite == 5 ? 3 : 5;
    }
}

/**
 * The function moves the player in the specified direction based on their
 * current state and updates their position and animation.
 *
 * @param direction The "direction" parameter is an enum type called "Direction"
 * which represents the direction in which the player should move. It can have
 * the following values:
 *
 * @return Nothing is being returned in this function.
 */
void Player::move(Direction direction) {
    if (dead_state || !GAME->playing) return;
    double distance = 0;
    if (ladder_state) {
        switch (direction) {
            case UP:
                y -= distance = speed * *delta * 80;
                delta_y -= distance;
                break;
            case DOWN:
                y += distance = speed * *delta * 80;
                delta_y += distance;
                break;
            default:
                break;
        }
    } else {
        switch (direction) {
            case RIGHT:
                x += distance = speed * *delta * 100;
                delta_x += distance;
                break;
            case LEFT:
                x -= distance = speed * *delta * 100;
                delta_x -= distance;
                break;
            default:
                break;
        }
    }

    this->direction = direction;
    if (distance) {
        moving = 1;
        animate();
    }
}

/**
 * The function creates level 1 of a game by placing various objects such as the
 * player, coins, barrels, platforms, and ladders in specific positions.
 *
 * @param objectList objectList is a pointer to an array of OBJECT pointers.
 * Each OBJECT pointer represents an object in the game level.
 * @param max The parameter "max" represents the maximum number of objects that
 * can be stored in the objectList array.
 * @param player The "player" parameter is a reference to an object of the
 * "Player" class. It represents the player character in the game.
 * @param barrelList The parameter `barrelList` is a pointer to an array of
 * `Barrel` objects.
 * @param barrelMax The parameter "barrelMax" represents the maximum number of
 * barrels that can be created in the level.
 */
void createLevel_1(OBJECT **objectList, int max, Player &player,
                   Barrel **barrelList, int barrelMax) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(-9, -5.5);

    PLACE(COIN, 0, 0);

    objectList[MONKE_]->place(6, 4.7);
    double path_x[] = {8.5, 1.5, -7.5, -10.5, -10.5, 100};
    double path_y[] = {0, -5, -6, -7, -7.5, 100};
    PLACE_BARREL(6, 4, path_x, path_y, 1);
    PLACE_BARREL(6, 4, path_x, path_y, 3);
    PLACE_BARREL(6, 4, path_x, path_y, 5);
    PLACE_BARREL(6, 4, path_x, path_y, 7);

    PLACE(PLATFORM_SHORT, -8, -7);
    PLACE(PLATFORM_MEDIUM, 6, -1);
    PLACE(LADDER_MEDIUM, 7, -1);
    PLACE(LADDER_TOP, 7, -1);
    PLACE(PLATFORM_MEDIUM, -6, -3);

    PLACE(PLATFORM_LONG, 1, -6);
    PLACE(PLATFORM_LONG, 0, 3);
    PLACE(LADDER_MEDIUM, 0, 3);
    PLACE(LADDER_TOP, 0, 3);

    PLACE(LADDER_SHORT, 3, 3);
    PLACE(LADDER_TOP, 3, 3);

    objectList[WIN_]->place(-6.5, 4);
}

/**
 * The function creates level 2 of a game by placing various objects and the
 * player character in specific positions.
 *
 * @param objectList objectList is a pointer to an array of OBJECT pointers.
 * Each OBJECT represents an object in the game level.
 * @param max The parameter "max" represents the maximum number of objects that
 * can be stored in the objectList array.
 * @param player The "player" parameter is a reference to an object of type
 * "Player". It represents the player character in the game.
 * @param barrelList The parameter "barrelList" is a pointer to an array of
 * pointers to Barrel objects.
 * @param barrelMax The parameter "barrelMax" represents the maximum number of
 * barrels that can be created in the level.
 */
void createLevel_2(OBJECT **objectList, int max, Player &player,
                   Barrel **barrelList, int barrelMax) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(10, -5.5);

    PLACE(PLATFORM_SHORT, 10, -7);
    PLACE(PLATFORM_SHORT, 2, -7);
    PLACE(PLATFORM_SHORT, -6, -7);
    PLACE(LADDER_MEDIUM, -12, -2);
    PLACE(LADDER_TOP, -12, -2);
    PLACE(PLATFORM_SHORT, -10, -2);

    PLACE(PLATFORM_LONG, 2, 1);
    PLACE(LADDER_SHORT, -4, 1);
    PLACE(LADDER_TOP, -4, 1);
    PLACE(LADDER_SHORT, -1, 1);
    PLACE(LADDER_TOP, -1, 1);
    PLACE(LADDER_SHORT, 2, 1);
    PLACE(LADDER_TOP, 2, 1);

    PLACE(PLATFORM_MEDIUM, -2, 4);

    PLACE(PLATFORM_SHORT, 10, 4);

    objectList[WIN_]->place(-9.5, 5.5);
}

/**
 * The function creates level 3 of a game by placing various objects and setting
 * their positions.
 *
 * @param objectList objectList is a pointer to an array of OBJECT pointers. It
 * is used to store the objects in the level.
 * @param max The parameter "max" represents the maximum number of objects that
 * can be stored in the objectList array.
 * @param player The "player" parameter is a reference to an object of type
 * "Player". It represents the player character in the game.
 * @param barrelList The parameter `barrelList` is a pointer to an array of
 * pointers to `Barrel` objects.
 * @param barrelMax The parameter "barrelMax" represents the maximum number of
 * barrels that can be placed in the level.
 */
void createLevel_3(OBJECT **objectList, int max, Player &player,
                   Barrel **barrelList, int barrelMax) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(10, -3.5);

    objectList[MONKE_]->place(-2, 3.7);

    double path_x0[] = {6.5, 8.5, 11.7, 100};
    double path_y0[] = {-3, -4, -7.5, 100};

    PLACE_BARREL(-2, 3, path_x0, path_y0, 1);
    PLACE_BARREL(-2, 3, path_x0, path_y0, 3);

    double path_x1[] = {-10.5, 5, 3, 2, 100};
    double path_y1[] = {0, -3, -4, -7.5, 100};

    PLACE_BARREL(-2, 3, path_x1, path_y1, 5);
    PLACE_BARREL(-2, 3, path_x1, path_y1, 7);

    PLACE(PLATFORM_SHORT, -2, 5);

    PLACE(PLATFORM_SHORT, 6, -4);

    PLACE(PLATFORM_MEDIUM, 7, -5);
    PLACE(PLATFORM_SHORT, -9, -6);

    PLACE(PLATFORM_SHORT, -1, -2.5);
    PLACE(PLATFORM_SHORT, -1, -3.5);
    PLACE(PLATFORM_SHORT, -1, -4.5);
    PLACE(PLATFORM_SHORT, -1, -5.5);
    PLACE(PLATFORM_SHORT, -1, -6.5);

    PLACE(PLATFORM_LONG, -4, -1);
    PLACE(LADDER_SHORT, -4, -1);
    PLACE(LADDER_TOP, -4, -1);
    PLACE(LADDER_SHORT, -9, -1);
    PLACE(LADDER_TOP, -9, -1);

    PLACE(PLATFORM_MEDIUM, -6, 2);
    PLACE(PLATFORM_MEDIUM, 2, 2);

    PLACE(LADDER_MEDIUM, -12, 4);
    PLACE(LADDER_TOP, -12, 4);

    PLACE(PLATFORM_SHORT, 10, 1);

    objectList[WIN_]->place(10.5, 2);
}

// main
#ifdef __cplusplus
extern "C"
#endif

    int
    main(int argc, char **argv) {
    int t1, t2, quit, frames, rc;
    double delta, worldTime, fpsTimer, fps;
    SDL_Event event;
    SDL_Surface *screen, *charset;
    SDL_Texture *scrtex;
    SDL_Window *window;
    SDL_Renderer *renderer;

    const Uint8 *keystate;

    SDL_OBJECTS_T sdl_obj;

    // okno konsoli nie jest widoczne, je�eli chcemy zobaczy�
    // komunikaty wypisywane printf-em trzeba w opcjach:
    // project -> szablon2 properties -> Linker -> System -> Subsystem
    // zmieni� na "Console"
    // console window is not visible, to see the printf output
    // the option:
    // project -> szablon2 properties -> Linker -> System -> Subsystem
    // must be changed to "Console"
    printf("wyjscie printfa trafia do tego okienka\n");
    printf("printf output goes here\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // tryb pe�noekranowy / fullscreen mode
    //	rc = SDL_CreateWindowAndRenderer(0, 0,
    // SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window,
                                     &renderer);
    if (rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;
    };
    sdl_obj.window = window;
    sdl_obj.renderer = renderer;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_SetWindowTitle(window,
                       "Wojciech Siwiec nr 197815 - Projekt 2 - King Donkey");

    sdl_obj.surfaces[sdl_obj.surface_index++] = screen =
        SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000,
                             0x0000FF00, 0x000000FF, 0xFF000000);

    sdl_obj.textures[sdl_obj.texture_index++] = scrtex = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    // wy��czenie widoczno�ci kursora myszy
    SDL_ShowCursor(SDL_DISABLE);

    // wczytanie obrazka cs8x8.bmp
    sdl_obj.surfaces[sdl_obj.surface_index++] = charset =
        SDL_LoadBMP("./cs8x8.bmp");
    if (charset == NULL) {
        printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
        free_sdl_resources(&sdl_obj);
        return 1;
    };
    SDL_SetColorKey(charset, true, 0x000000);

    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
    int granatowy = SDL_MapRGB(screen->format, 0x00, 0x00, 0x80);

    t1 = SDL_GetTicks();

    frames = 0;
    fpsTimer = 0;
    fps = 0;
    quit = 0;
    worldTime = 0;

    /* The code below is loading images into surfaces for different sprite
    sheets. It is using a loop to load multiple images for each sprite sheet.
    The images are loaded from different directories based on the value of the
    loop variable. If the loading of any image fails, the function returns 1. */
    SPRITESHEET_T palyerSheet;
    for (int i = 0; i < 9; i++) {
        sprintf(text, "Player/%d", i);
        if (load_image_into_surface(&(palyerSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }
    SPRITESHEET_T ladderSheet;
    for (int i = 0; i < 4; i++) {
        sprintf(text, "Ladder/%d", i);
        if (load_image_into_surface(&(ladderSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }
    SPRITESHEET_T barrelSheet;
    for (int i = 0; i < 6; i++) {
        sprintf(text, "Barrel/%d", i);
        if (load_image_into_surface(&(barrelSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }
    SPRITESHEET_T platformSheet;
    for (int i = 0; i < 3; i++) {
        sprintf(text, "Platform/%d", i);
        if (load_image_into_surface(&(platformSheet.sprite[i]), text,
                                    &sdl_obj) != 0) {
            return 1;
        }
    }
    SPRITESHEET_T winSheet;
    for (int i = 0; i < 2; i++) {
        sprintf(text, "Win/%d", i);
        if (load_image_into_surface(&(winSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }

    SPRITESHEET_T monkeSheet;
    for (int i = 0; i < 4; i++) {
        sprintf(text, "Monke/%d", i);
        if (load_image_into_surface(&(monkeSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }
    SPRITESHEET_T coinSheet;
    for (int i = 0; i < 1; i++) {
        sprintf(text, "Coin/%d", i);
        if (load_image_into_surface(&(coinSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }

    /* The code below is creating and initializing various objects and lists for
     * a game. */
    OBJECT *objectList[MAX_OBJECTS];
    int objectListMaxIndex = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            objectList[objectListMaxIndex++] =
                new OBJECT(PLATFORM, i, &delta, &platformSheet);
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            objectList[objectListMaxIndex++] =
                new OBJECT(LADDER, i, &delta, &ladderSheet);
        }
    }
    for (int i = 0; i < 10; i++) {
        objectList[objectListMaxIndex++] =
            new OBJECT(LADDER_TOP, 0, &delta, &ladderSheet);
    }

    for (int i = 0; i < 10; i++) {
        objectList[objectListMaxIndex++] =
            new OBJECT(COIN, 0, &delta, &coinSheet);
    }

    TextPopup *textPopupList[MAX_POPUPS];
    int textPopupListMaxIndex = 0;

    for (int i = 0; i < 10; i++) {
        textPopupList[textPopupListMaxIndex++] = new TextPopup(&delta, charset);
    }

    objectList[objectListMaxIndex++] = new OBJECT(WIN, 0, &delta, &winSheet);

    int monkey_index = objectListMaxIndex++;
    objectList[monkey_index] = new OBJECT(MONKE, 0, &delta, &monkeSheet);

    Barrel *barrelList[MAX_BARRELS];
    int barrelListMaxIndex = 0;
    for (int i = 0; i < 10; i++) {
        barrelList[barrelListMaxIndex++] =
            new Barrel(&delta, &barrelSheet, objectList[monkey_index]);
    }

    GAME_T GAME;

    Player player(&delta, &palyerSheet, objectList, objectListMaxIndex, &GAME,
                  barrelList, barrelListMaxIndex, textPopupList);
    player.place(0, 0);

    createLevel_1(objectList, objectListMaxIndex, player, barrelList,
                  barrelListMaxIndex);

    while (!quit) {
        t2 = SDL_GetTicks();

        // w tym momencie t2-t1 to czas w milisekundach,
        // jaki uplyna� od ostatniego narysowania ekranu
        // delta to ten sam czas w sekundach
        // here t2-t1 is the time in milliseconds since
        // the last screen was drawn
        // delta is the same time in seconds
        delta = (t2 - t1) * 0.001;
        t1 = t2;

        if (GAME.playing) worldTime += delta;

        SDL_FillRect(screen, NULL, granatowy);

        DrawRectangle(screen, start_x, start_y, end_x, end_y, niebieski,
                      czarny);

        fpsTimer += delta;
        if (fpsTimer > 0.5) {
            fps = frames * 2;
            frames = 0;
            fpsTimer -= 0.5;
        };
        sprintf(text, "Zaimplementowane: A, B, C, E, F");
        DrawString(screen, start_x, start_y - 16, text, charset);

        sprintf(text,
                "Interfejs gry: WIN:%d LOSE:%d SCORE:%d LIVES:%d LEVEL:%d",
                GAME.win, GAME.lose, GAME.score, GAME.lives, GAME.level);
        DrawString(screen, start_x + 8, SCREEN_HEIGHT - 32, text, charset);

        // tekst informacyjny / info text
        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
        //            "template for the second project, elapsed time = %.1lf
        //            s
        //            %.0lf frames / s"

        sprintf(text,
                "Wojciech Siwiec nr 197815, czas trwania = %.1lfs%s"
                " | %.0lf/fps ",
                worldTime, (GAME.playing ? "" : " STOPED"), fps);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text,
                   charset);
        //	      "Esc - exit, \030 - faster, \031 - slower"
        sprintf(text,
                "Esc - exit, n - nowa rozgrywka, (1|2|3) - wybor poziomu");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text,
                   charset);

        /* The code below is rendering a game scene. */
        if (!GAME.playing) objectList[WIN_]->simple_animation(1, 0, 1, 0);
        objectList[MONKE_]->simple_animation(1, 0, 3, 1);

        for (int i = 0; i < objectListMaxIndex; i++) {
            if (objectList[i] != NULL) objectList[i]->draw(screen);
        }
        for (int i = 0; i < barrelListMaxIndex; i++) {
            if (barrelList[i] != NULL) barrelList[i]->nextFrame(screen);
        }

        player.nextFrame(screen);

        for (int i = 0; i < textPopupListMaxIndex; i++) {
            if (textPopupList[i] != NULL) textPopupList[i]->nextFrame(screen);
        }

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //		SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        // obs�uga zdarze� (o ile jakie� zasz�y) / handling of events (if
        // there were any)
        int build_level = 0;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        quit = 1;
                    else if (event.key.keysym.sym == SDLK_n) {
                        worldTime = 0;
                        GAME.score = 0;
                        GAME.lives = 3;
                        GAME.win = 0;
                        GAME.lose = 0;
                        player.reset();
                        build_level = 1;
                    } else if (event.key.keysym.sym == SDLK_1) {
                        GAME.level = 1;
                        build_level = 1;
                    } else if (event.key.keysym.sym == SDLK_2) {
                        GAME.level = 2;
                        build_level = 1;
                    } else if (event.key.keysym.sym == SDLK_3) {
                        GAME.level = 3;
                        build_level = 1;
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
            };
        };
        if (build_level) {
            GAME.playing = 1;
            switch (GAME.level) {
                case 1:
                    createLevel_1(objectList, objectListMaxIndex, player,
                                  barrelList, barrelListMaxIndex);
                    break;
                case 2:
                    createLevel_2(objectList, objectListMaxIndex, player,
                                  barrelList, barrelListMaxIndex);
                    break;
                case 3:
                    createLevel_3(objectList, objectListMaxIndex, player,
                                  barrelList, barrelListMaxIndex);
                    break;
            }
        }

        /* The code below is checking the state of the keyboard using
        SDL_GetKeyboardState() function. It then checks if certain keys (right
        arrow, left arrow, D, A, up arrow, W, down arrow, S, and space) are
        pressed. Depending on which keys are pressed, it sets the player's
        ladder state and calls the move() or jump() functions accordingly. */
        player.moving = 0;
        keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
            player.ladder_state = 0;
            player.move(RIGHT);
        }
        if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
            player.ladder_state = 0;
            player.move(LEFT);
        }
        if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W]) {
            if (player.ladder_possible)
                player.ladder_state = 1;
            else
                player.ladder_state = 0;
            player.move(UP);
        }
        if (keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S]) {
            if (player.ladder_possible)
                player.ladder_state = 1;
            else
                player.ladder_state = 0;
            if (player.ladder_top) player.ladder_state = 1;
            player.move(DOWN);
        }
        if (keystate[SDL_SCANCODE_SPACE]) {
            player.jump();
        }

        frames++;

        // // Sztuczne opóźnienie
        // for (int i = 0; i < 1000; i++) printf("  \b\b");

        player.animate();
    };

    // zwolnienie powierzchni / freeing all surfaces
    free_sdl_resources(&sdl_obj);

    return 0;
};

// ∴𝙹⋮ᓵ╎ᒷᓵ⍑ ᓭ╎∴╎ᒷᓵ
// ᔑꖎᒷ ⎓ᔑ⋮リ╎ᒷ ᓭ╎ᒷ !¡╎ᓭ⨅ᒷ ℸ ̣ ᒷ ꖌ𝙹↸||, ⨅ᔑᒲ╎ᔑᓭℸ ̣  ⚍ᓵ⨅||ᓵ ᓭ╎ᒷ ᒲᔑℸ ̣ ᒷᒲᔑℸ ̣ ||ꖌ╎.
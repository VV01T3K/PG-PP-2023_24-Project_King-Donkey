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
    double frameCounter = 0;
    int curent_sprite = 0;
    int anim_cycle = 0;
    int type;
    double *delta;
    double x = 0;
    double y = 0;
    START_VALUES_OBJECT_T start_values;

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
};
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
void OBJECT::reset() {
    x = start_values.x;
    y = start_values.y;
    curent_sprite = start_values.curent_sprite;
    anim_cycle = start_values.anim_cycle;
}
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
void OBJECT::draw(SDL_Surface *screen) {
    if (x == 0 && y == 0) return;
    DrawSurface(screen, sheet->sprite[curent_sprite], x, y);
    frameCounter += *delta * 200;
    if (frameCounter > 1000) frameCounter = 0;
}
class Player : public OBJECT {
   private:
    OBJECT **objectList;
    int objectListSize;
    Direction direction = RIGHT;
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
           int objectListSize, GAME_T *GAME)
        : OBJECT(0, 0, delta, sheet),
          objectList(objectList),
          objectListSize(objectListSize),
          GAME(GAME) {
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
};
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
void Player::jump() {
    if (jump_state || ladder_state || dead_state || falling) return;
    jump_state = 1;
    gravity = max_gravity * -(JUMP_HEIGHT)*speed;
}
void Player::nextFrame(SDL_Surface *screen) {
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
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                getBORDER(LEFT) < objectList[i]->getBORDER(RIGHT) &&
                getBORDER(RIGHT) > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
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
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                getBORDER(LEFT) < objectList[i]->getBORDER(RIGHT) &&
                getBORDER(RIGHT) > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
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
        if (objectList[i]->type == ENEMY) {
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                getBORDER(LEFT) < objectList[i]->getBORDER(RIGHT) &&
                getBORDER(RIGHT) > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
                UNALIVE = 1;
            }
        }
        if (objectList[i]->type == WIN) {
            if (getBORDER(DOWN) > objectList[i]->getBORDER(UP) &&
                getBORDER(LEFT) < objectList[i]->getBORDER(RIGHT) &&
                getBORDER(RIGHT) > objectList[i]->getBORDER(LEFT) &&
                getBORDER(UP) < objectList[i]->getBORDER(DOWN)) {
                GAME->win += 1;
            }
        }
    }

    if (zatrzymantko && delta_y > 0) y -= delta_y;

    // EVENTS
    if (UNALIVE) dead_state = 1;
    if (horizontalSTOP) x -= delta_x;
    if (verticalSTOP) y -= delta_y;
}
void Player::animate() {
    if (dead_state) {
        curent_sprite = 8;
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
void Player::move(Direction direction) {
    if (dead_state) return;
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
void createLevel_1(OBJECT **objectList, int max, Player &player) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(-10, -5.5);
    objectList[NEXT(PLATFORM_SHORT)]->place(-10, -7);
    objectList[NEXT(PLATFORM_MEDIUM)]->place(6, -1);
    objectList[NEXT(LADDER_MEDIUM)]->place(7, -1);
    objectList[NEXT(LADDER_TOP)]->place(7, -1);
    objectList[NEXT(PLATFORM_MEDIUM)]->place(-6, -3);

    objectList[NEXT(PLATFORM_LONG)]->place(0, -6);
    objectList[NEXT(PLATFORM_LONG)]->place(0, 3);
    objectList[NEXT(LADDER_MEDIUM)]->place(0, 3);
    objectList[NEXT(LADDER_TOP)]->place(0, 3);

    objectList[NEXT(LADDER_SHORT)]->place(3, 3);
    objectList[NEXT(LADDER_TOP)]->place(3, 3);

    objectList[WIN_]->place(-6.5, 4);
}
void createLevel_2(OBJECT **objectList, int max, Player &player) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(10, -5.5);
    objectList[NEXT(PLATFORM_SHORT)]->place(10, -7);
    objectList[NEXT(PLATFORM_SHORT)]->place(2, -7);
    objectList[NEXT(PLATFORM_SHORT)]->place(-6, -7);
    objectList[NEXT(LADDER_MEDIUM)]->place(-12, -2);
    objectList[NEXT(LADDER_TOP)]->place(-12, -2);
    objectList[NEXT(PLATFORM_SHORT)]->place(-10, -2);

    objectList[NEXT(PLATFORM_LONG)]->place(2, 1);
    objectList[NEXT(LADDER_SHORT)]->place(-4, 1);
    objectList[NEXT(LADDER_TOP)]->place(-4, 1);
    objectList[NEXT(LADDER_SHORT)]->place(-1, 1);
    objectList[NEXT(LADDER_TOP)]->place(-1, 1);
    objectList[NEXT(LADDER_SHORT)]->place(2, 1);
    objectList[NEXT(LADDER_TOP)]->place(2, 1);

    objectList[NEXT(PLATFORM_MEDIUM)]->place(-2, 4);

    objectList[NEXT(PLATFORM_SHORT)]->place(10, 4);

    objectList[WIN_]->place(-9.5, 5.5);
}
void createLevel_3(OBJECT **objectList, int max, Player &player) {
    __BUILDLEVEL_PREPARE__

    // MAX 10 objects of the same type
    // NEXT(OBJECT_TYPE) | place(x, y) each TILE_SIZE
    player.place(-7, -5.5);

    objectList[NEXT(PLATFORM_LONG)]->place(4, -2);
    objectList[NEXT(PLATFORM_SHORT)]->place(-9, -3);
    objectList[NEXT(LADDER_MEDIUM)]->place(7, -2);
    objectList[NEXT(LADDER_TOP)]->place(7, -2);

    objectList[NEXT(PLATFORM_LONG)]->place(0, -7);

    objectList[NEXT(PLATFORM_LONG)]->place(-4, 2);
    objectList[NEXT(LADDER_SHORT)]->place(-4, 2);
    objectList[NEXT(LADDER_TOP)]->place(-4, 2);
    objectList[NEXT(LADDER_SHORT)]->place(1, 2);
    objectList[NEXT(LADDER_TOP)]->place(1, 2);

    objectList[NEXT(PLATFORM_MEDIUM)]->place(-6, 5);
    objectList[NEXT(PLATFORM_MEDIUM)]->place(2, 5);

    objectList[NEXT(LADDER_SHORT)]->place(-12, 5);
    objectList[NEXT(LADDER_TOP)]->place(-12, 5);

    objectList[NEXT(PLATFORM_SHORT)]->place(10, 4);

    objectList[WIN_]->place(10.5, 5);
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
    for (int i = 0; i < 4; i++) {
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
    for (int i = 0; i < 1; i++) {
        sprintf(text, "Win/%d", i);
        if (load_image_into_surface(&(winSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }

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

    // for (int i = 0; i < 10; i++) {
    //     objectList[objectListMaxIndex++] =
    //         new OBJECT(ENEMY, 0, &delta, &barrelSheet);
    // }

    for (int i = 0; i < 1; i++) {
        objectList[objectListMaxIndex++] =
            new OBJECT(WIN, 0, &delta, &winSheet);
    }

    GAME_T GAME;

    Player player(&delta, &palyerSheet, objectList, objectListMaxIndex, &GAME);
    player.place(0, 0);

    createLevel_3(objectList, objectListMaxIndex, player);

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

        worldTime += delta;

        SDL_FillRect(screen, NULL, granatowy);

        DrawRectangle(screen, start_x, start_y, end_x, end_y, niebieski,
                      czarny);

        fpsTimer += delta;
        if (fpsTimer > 0.5) {
            fps = frames * 2;
            frames = 0;
            fpsTimer -= 0.5;
        };
        sprintf(text, "Zaimplementowane: ");
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
                "Wojciech Siwiec nr 197815, czas trwania = %.1lfs"
                " | %.0lf/fps ",
                worldTime, fps);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text,
                   charset);
        //	      "Esc - exit, \030 - faster, \031 - slower"
        sprintf(text,
                "Esc - exit, n - nowa rozgrywka, (1|2|3) - wybor poziomu");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text,
                   charset);

        // rysowanie obiekt�w / drawing objects
        for (int i = 0; i < objectListMaxIndex; i++) {
            if (objectList[i] != NULL) objectList[i]->draw(screen);
        }

        player.nextFrame(screen);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //		SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        // obs�uga zdarze� (o ile jakie� zasz�y) / handling of events (if
        // there were any)

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        quit = 1;
                    else if (event.key.keysym.sym == SDLK_n) {
                        worldTime = 0;
                        player.reset();
                        for (int i = 0; i < objectListMaxIndex; i++) {
                            if (objectList[i] != NULL) objectList[i]->reset();
                        }
                    } else if (event.key.keysym.sym == SDLK_1) {
                        createLevel_1(objectList, objectListMaxIndex, player);
                    } else if (event.key.keysym.sym == SDLK_2) {
                        createLevel_2(objectList, objectListMaxIndex, player);
                    } else if (event.key.keysym.sym == SDLK_3) {
                        createLevel_3(objectList, objectListMaxIndex, player);
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
            };
        };
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
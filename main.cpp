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

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} BORDERS_T;

class OBJECT {
   private:
    SPRITESHEET_T *sheet;
    double *frameCounter;
    int curent_sprite = 0;
    int anim_cycle = 0;

   public:
    double x = 0;
    double y = 0;
    int type;

    // Default constructor
    OBJECT() {}
    // Overloaded constructor
    OBJECT(int type, double x, double y, SPRITESHEET_T *sheet,
           double *frameCounter)
        : type(type), x(x), y(y), sheet(sheet), frameCounter(frameCounter) {
        if (type == LADDER_TOP) this->curent_sprite = 1;
    }

    void draw(SDL_Surface *screen);
    double getBORDER(Direction side);
};
double OBJECT::getBORDER(Direction side) {
    switch (side) {
        case RIGHT:
            return this->x + this->sheet->sprite[0]->w / 2;
        case LEFT:
            return this->x - this->sheet->sprite[0]->w / 2;
        case UP:
            return this->y - this->sheet->sprite[0]->h / 2;
        case DOWN:
            return this->y + this->sheet->sprite[0]->h / 2;
        default:
            return 0;
    }
}
void OBJECT::draw(SDL_Surface *screen) {
    DrawSurface(screen, this->sheet->sprite[this->curent_sprite], this->x,
                this->y);
}
class Player {
   private:
    SPRITESHEET_T *sheet;
    OBJECT **objectList;
    int objectListSize;
    double *delta;
    double *frameCounter;
    Direction direction = RIGHT;
    double speed = 1.5;
    int curent_sprite = 0;
    int anim_cycle = 0;
    double max_gravity = 4;
    double gravity;
    double gravity_delta = 0;
    double delta_x = 0;
    double delta_y = 0;

   public:
    int ladder_possible = 0;
    int ladder_state = 0;
    int ladder_top = 0;
    int dead_state = 0;
    int jump_state = 0;
    int moving = 0;
    double x = 0;
    double y = 0;
    // void jump();
    // void gravity();
    Player(double x, double y, double *delta, SPRITESHEET_T *sheet,
           double *frameCounter, OBJECT **objectList, int objectListSize)
        : objectList(objectList),
          objectListSize(objectListSize),
          delta(delta),
          sheet(sheet),
          frameCounter(frameCounter),
          x(x),
          y(y) {
        this->gravity = this->max_gravity;
    }

    void draw(SDL_Surface *screen);
    void move(Direction direction);
    void animate();
    void collision();
    void nextFrame(SDL_Surface *screen);
    double getBORDER(Direction side);
    void jump();
};
void Player::jump() {
    if (this->jump_state || this->ladder_state || this->dead_state) return;
    this->jump_state = 1;
    this->gravity = this->max_gravity * -(JUMP_HEIGHT);
}
double Player::getBORDER(Direction side) {
    switch (side) {
        case RIGHT:
            return this->x + this->sheet->sprite[0]->w / 2;
        case LEFT:
            return this->x - this->sheet->sprite[0]->w / 2;
        case UP:
            return this->y - this->sheet->sprite[0]->h / 2;
        case DOWN:
            return this->y + this->sheet->sprite[0]->h / 2;
        default:
            return 0;
    }
}
void Player::nextFrame(SDL_Surface *screen) {
    if (this->gravity < max_gravity || !this->jump_state)
        this->gravity += *delta * max_gravity * 4;

    double gravity_distance = this->gravity * *delta * 100;

    this->gravity_delta = 0;
    if (this->ladder_state == 0) {
        this->gravity_delta = gravity_distance;
        this->y += gravity_distance;
    }
    this->ladder_possible = 0;
    this->ladder_top = 0;

    this->collision();

    this->draw(screen);
    this->delta_x = 0;
    this->delta_y = 0;
}

void Player::collision() {
    int horizontalSTOP = 0, verticalSTOP = 0;
    int UNALIVE = 0;
    // check for collision with screen borders
    if (this->getBORDER(LEFT) < start_x) horizontalSTOP = 1;
    if (this->getBORDER(RIGHT) > end_x + start_x) horizontalSTOP = 1;
    if (this->getBORDER(UP) < start_y) verticalSTOP = 1;
    if (this->getBORDER(DOWN) > end_y + start_y) {
        verticalSTOP = 1;
        this->y -= this->gravity_delta;
        this->jump_state = 0;
    }

    // check for collision with objects
    for (int i = 0; i < this->objectListSize; i++) {
        if (this->objectList[i] == NULL) continue;
        if (this->objectList[i]->type == NOTHING) continue;
        if (this->objectList[i]->type == PLATFORM) {
            if (this->getBORDER(DOWN) > this->objectList[i]->getBORDER(UP) &&
                this->getBORDER(LEFT) < this->objectList[i]->getBORDER(RIGHT) &&
                this->getBORDER(RIGHT) > this->objectList[i]->getBORDER(LEFT) &&
                this->getBORDER(UP) < this->objectList[i]->getBORDER(DOWN)) {
                this->y -= this->gravity_delta;
                this->jump_state = 0;
            }
            if (this->getBORDER(DOWN) > this->objectList[i]->getBORDER(UP) &&
                this->getBORDER(LEFT) < this->objectList[i]->getBORDER(RIGHT) &&
                this->getBORDER(RIGHT) > this->objectList[i]->getBORDER(LEFT) &&
                this->getBORDER(UP) < this->objectList[i]->getBORDER(DOWN)) {
                horizontalSTOP = 1;
                this->y += this->gravity_delta;
            }
        }
        if (this->objectList[i]->type == LADDER) {
            if (this->getBORDER(DOWN) > this->objectList[i]->getBORDER(UP) &&
                this->x < this->objectList[i]->getBORDER(RIGHT) &&
                this->x > this->objectList[i]->getBORDER(LEFT) &&
                this->getBORDER(UP) < this->objectList[i]->getBORDER(DOWN)) {
                this->ladder_possible = 1;
            }
        }
        if (this->objectList[i]->type == LADDER_TOP) {
            if (this->getBORDER(DOWN) > this->objectList[i]->getBORDER(UP) &&
                this->x < this->objectList[i]->getBORDER(RIGHT) &&
                this->x > this->objectList[i]->getBORDER(LEFT) &&
                this->getBORDER(UP) < this->objectList[i]->getBORDER(DOWN)) {
                this->ladder_top = 1;
            }
        }
        if (this->objectList[i]->type == ENEMY) {
            if (this->getBORDER(DOWN) > this->objectList[i]->getBORDER(UP) &&
                this->getBORDER(LEFT) < this->objectList[i]->getBORDER(RIGHT) &&
                this->getBORDER(RIGHT) > this->objectList[i]->getBORDER(LEFT) &&
                this->getBORDER(UP) < this->objectList[i]->getBORDER(DOWN)) {
                UNALIVE = 1;
            }
        }
    }

    // EVENTS
    if (!this->jump_state) this->gravity = this->max_gravity / 2;
    if (UNALIVE) this->dead_state = 1;
    if (horizontalSTOP) this->x -= this->delta_x;
    if (verticalSTOP) this->y -= this->delta_y;
}
void Player::draw(SDL_Surface *screen) {
    DrawSurface(screen, this->sheet->sprite[this->curent_sprite], this->x,
                this->y);
}
void Player::animate() {
    if (this->dead_state) {
        this->curent_sprite = 8;
        return;
    }
    if (this->jump_state) {
        this->curent_sprite = 1;
        if (this->direction == RIGHT) this->curent_sprite = 4;

        return;
    }

    if (this->ladder_state) {
        if (*frameCounter >= 80 && this->moving) {
            *frameCounter = 0;
            this->curent_sprite = this->curent_sprite == 7 ? 6 : 7;
        } else if (!this->moving) {
            this->curent_sprite = 6;
        }

        return;
    }

    if (this->moving == 0) {
        this->curent_sprite = 0;
        if (this->direction == RIGHT) this->curent_sprite = 3;

        return;
    }
    if (*frameCounter >= 30) {
        *frameCounter = 0;
        if (this->direction == LEFT)
            this->curent_sprite = this->curent_sprite == 2 ? 0 : 2;
        else
            this->curent_sprite = this->curent_sprite == 5 ? 3 : 5;
    }
}
void Player::move(Direction direction) {
    if (this->dead_state) return;
    double distance = 0;
    if (this->ladder_state) {
        switch (direction) {
            case UP:
                this->y -= distance = this->speed * *delta * 80;
                this->delta_y -= distance;
                break;
            case DOWN:
                this->y += distance = this->speed * *delta * 80;
                this->delta_y += distance;
                break;
            default:
                break;
        }
    } else {
        switch (direction) {
            case RIGHT:
                this->x += distance = this->speed * *delta * 100;
                this->delta_x += distance;
                break;
            case LEFT:
                this->x -= distance = this->speed * *delta * 100;
                this->delta_x -= distance;
                break;
            default:
                break;
        }
    }

    this->direction = direction;
    if (distance) {
        this->moving = 1;
        this->animate();
    }
}

// main
#ifdef __cplusplus
extern "C"
#endif

    int
    main(int argc, char **argv) {
    int t1, t2, quit, frames, rc;
    double frameCounter = 0;
    double delta, worldTime, fpsTimer, fps, distance;
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

    SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");

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
    distance = 0;

    OBJECT *objectList[MAX_OBJECTS];
    int objectListMaxIndex = 0;

    SPRITESHEET_T etiSheet;
    if (load_image_into_surface(&(etiSheet.sprite[0]), "eti", &sdl_obj) != 0) {
        return 1;
    }

    SPRITESHEET_T palyerSheet;
    for (int i = 0; i < 9; i++) {
        sprintf(text, "Player/%d", i);
        if (load_image_into_surface(&(palyerSheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }
    SPRITESHEET_T ladderSheet;
    for (int i = 0; i < 3; i++) {
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

    objectList[objectListMaxIndex++] =
        new OBJECT(PLATFORM, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.25,
                   &platformSheet, &frameCounter);

    objectList[objectListMaxIndex++] =
        new OBJECT(LADDER, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.2, &ladderSheet,
                   &frameCounter);
    objectList[objectListMaxIndex++] =
        new OBJECT(LADDER, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.15, &ladderSheet,
                   &frameCounter);
    objectList[objectListMaxIndex++] =
        new OBJECT(LADDER, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.25, &ladderSheet,
                   &frameCounter);
    objectList[objectListMaxIndex++] =
        new OBJECT(LADDER_TOP, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.30,
                   &ladderSheet, &frameCounter);

    objectList[objectListMaxIndex++] =
        new OBJECT(ENEMY, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 1.1, &barrelSheet,
                   &frameCounter);

    Player player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3, &delta, &palyerSheet,
                  &frameCounter, objectList, objectListMaxIndex);

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

        sprintf(text, "Interfejs gry...");
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
                        distance = 0;
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
        frameCounter += delta * 200;
        if (frameCounter > 1000) frameCounter = 0;

        // // Sztuczne opóźnienie
        // for (int i = 0; i < 1000; i++) printf("  \b\b");

        player.animate();
    };

    // zwolnienie powierzchni / freeing all surfaces
    free_sdl_resources(&sdl_obj);

    return 0;
};
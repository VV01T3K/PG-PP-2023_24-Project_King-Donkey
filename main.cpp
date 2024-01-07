#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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

#define MAX_OBJECTS 30
typedef struct {
    SDL_Surface *sprite[9];
} SPRITESHEET_T;
typedef struct {
    SDL_Surface *surfaces[MAX_OBJECTS];
    SDL_Texture *textures[MAX_OBJECTS];
    SDL_Window *window;
    SDL_Renderer *renderer;
    int surface_index = 0;
    int texture_index = 0;
} SDL_OBJECTS_T;

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
enum Direction { RIGHT, LEFT, UP, DOWN };
class Player {
   private:
    int collison_state = 0;
    double speed = 1.5;
    double *delta;
    int jump_state = 0;
    SPRITESHEET_T *sheet;
    int curent_sprite = 0;
    Direction direction = RIGHT;
    double *frameCounter;

   public:
    int ladder_state = 0;
    int dead_state = 0;
    int moving = 0;
    double x = 0;
    double y = 0;
    // void jump();
    // void gravity();
    // void collision();
    Player(int x, int y, double *delta, SPRITESHEET_T *sheet,
           double *frameCounter) {
        this->x = (double)x;
        this->y = (double)y;
        this->delta = delta;
        this->sheet = sheet;
        this->frameCounter = frameCounter;
    }
    void draw(SDL_Surface *screen);
    void move(Direction direction);
    void animate();
};
void Player::draw(SDL_Surface *screen) {
    DrawSurface(screen, this->sheet->sprite[this->curent_sprite], this->x,
                this->y);
}
void Player::animate() {
    if (this->dead_state) {
        this->curent_sprite = 8;
        return;
    }

    if (this->ladder_state) {
        if (*frameCounter >= 80 && this->moving) {
            *frameCounter = 0;
            this->curent_sprite++;
            if (this->curent_sprite > 7) {
                this->curent_sprite = 6;
            }
        } else if (!this->moving) {
            this->curent_sprite = 6;
        }

        return;
    }

    if (this->moving == 0) {
        this->curent_sprite = 0;
        if (this->direction == RIGHT) {
            this->curent_sprite = 3;
        }
        return;
    }
    if (*frameCounter >= 30) {
        *frameCounter = 0;
        if (this->direction == LEFT) {
            this->curent_sprite++;
            if (this->curent_sprite > 1) {
                this->curent_sprite = 0;
            }
        } else if (this->direction == RIGHT) {
            if (this->curent_sprite < 3) this->curent_sprite = 3;
            this->curent_sprite++;
            if (this->curent_sprite > 4) {
                this->curent_sprite = 3;
            }
        }
    }
}
void Player::move(Direction direction) {
    if (this->dead_state) return;

    if (this->ladder_state) {
        switch (direction) {
            case UP:
                this->y -= this->speed * *delta * 80;
                break;
            case DOWN:
                this->y += this->speed * *delta * 80;
                break;
            default:
                break;
        }
    } else {
        switch (direction) {
            case RIGHT:
                this->x += this->speed * *delta * 100;
                break;
            case LEFT:
                this->x -= this->speed * *delta * 100;
                break;
            default:
                break;
        }
    }
    this->direction = direction;
    this->moving = 1;
    this->animate();
}

// main
#ifdef __cplusplus
extern "C"
#endif

    int
    main(int argc, char **argv) {
    int t1, t2, quit, frames, rc;
    double frameCounter = 0;
    double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
    SDL_Event event;
    SDL_Surface *screen, *charset;
    SDL_Surface *eti;
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
    //	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
    //	                                 &window, &renderer);
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

    if (load_image_into_surface(&eti, "eti", &sdl_obj) != 0) {
        return 1;
    }

    char text[128];
    SPRITESHEET_T spritesheet;
    for (int i = 0; i < 9; i++) {
        sprintf(text, "Player/%d", i);
        if (load_image_into_surface(&(spritesheet.sprite[i]), text, &sdl_obj) !=
            0) {
            return 1;
        }
    }

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
    etiSpeed = 1;

    Player player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, &delta, &spritesheet,
                  &frameCounter);

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

        distance += etiSpeed * delta;

        SDL_FillRect(screen, NULL, granatowy);

        const int padding_top = 24;
        const int padding_bottom = 16;
        const int padding_left = 16;
        const int padding_right = 16;

        const int start_x = padding_left;
        const int start_y = 40 + padding_top;
        const int end_x = SCREEN_WIDTH - (padding_left + padding_right);
        const int end_y =
            SCREEN_HEIGHT - (32 + 40 + padding_top + padding_bottom);

        DrawRectangle(screen, start_x, start_y, end_x, end_y, niebieski,
                      czarny);

        DrawSurface(screen, eti,
                    SCREEN_WIDTH / 2 + sin(distance) * SCREEN_HEIGHT / 3,
                    SCREEN_HEIGHT / 2 + cos(distance) * SCREEN_HEIGHT / 3);

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
        //            "template for the second project, elapsed time = %.1lf s
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

        player.draw(screen);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //		SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        // obs�uga zdarze� (o ile jakie� zasz�y) / handling of events (if there
        // were any)

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        quit = 1;
                    else if (event.key.keysym.sym == SDLK_n) {
                        worldTime = 0;
                        distance = 0;
                    } else if (event.key.keysym.sym == SDLK_1) {
                        player.ladder_state = player.ladder_state ? 0 : 1;
                    } else if (event.key.keysym.sym == SDLK_2) {
                        player.dead_state = player.dead_state ? 0 : 1;
                    }
                    // else if (event.key.keysym.sym == SDLK_3) {
                    // }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
            };
        };
        player.moving = 0;
        keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
            player.move(RIGHT);
        }
        if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
            player.move(LEFT);
        }
        if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W]) {
            player.move(UP);
        }
        if (keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S]) {
            player.move(DOWN);
        }

        frames++;
        frameCounter += delta * 200;
        if (frameCounter > 1000) frameCounter = 0;

        player.animate();
    };

    // zwolnienie powierzchni / freeing all surfaces
    free_sdl_resources(&sdl_obj);

    return 0;
};
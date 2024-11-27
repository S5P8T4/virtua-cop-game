#include <SDL.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_WIDTH 30
#define PLAYER_HEIGHT 50
#define BULLET_WIDTH 5
#define BULLET_HEIGHT 10
#define TARGET_RADIUS 20
#define MAX_BULLETS 10
#define MAX_TARGETS 5
#define MAX_TARGET_SPEED 1

typedef struct {
    int x, y, width, height;
} Player;

typedef struct {
    int x, y, speed;
    int active;
} Bullet;

typedef struct {
    int x, y, speedX, speedY, radius;
    int hit;
} Target;

SDL_Renderer* renderer;
Player player;
Bullet bullets[MAX_BULLETS];
Target targets[MAX_TARGETS];

void initPlayer() {
    player.x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    player.y = SCREEN_HEIGHT - PLAYER_HEIGHT - 10;
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
}

void initBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }
}

void generateTargets() {
    for (int i = 0; i < MAX_TARGETS; i++) {
        targets[i].x = rand() % (SCREEN_WIDTH - TARGET_RADIUS * 2) + TARGET_RADIUS;
        targets[i].y = rand() % (SCREEN_HEIGHT / 2);
        targets[i].speedX = rand() % MAX_TARGET_SPEED + 1;
        targets[i].speedY = rand() % MAX_TARGET_SPEED + 1;
        targets[i].radius = TARGET_RADIUS;
        targets[i].hit = 0;
    }
}

void shootBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x + PLAYER_WIDTH / 2 - BULLET_WIDTH / 2;
            bullets[i].y = player.y;
            bullets[i].speed = 5;
            bullets[i].active = 1;
            break;
        }
    }
}

void movePlayer(SDL_Event* e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_LEFT:
            player.x -= 10;
            break;
        case SDLK_RIGHT:
            player.x += 10;
            break;
        }
        if (player.x < 0) player.x = 0;
        if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) player.x = SCREEN_WIDTH - PLAYER_WIDTH;
    }
}

void moveBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= bullets[i].speed;
            if (bullets[i].y < 0) {
                bullets[i].active = 0;
            }
        }
    }
}

void moveTargets() {
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (!targets[i].hit) {
            targets[i].x += targets[i].speedX;
            targets[i].y += targets[i].speedY;

            if (targets[i].x <= 0 || targets[i].x >= SCREEN_WIDTH - targets[i].radius * 2) {
                targets[i].speedX = -targets[i].speedX;
            }
            if (targets[i].y <= 0 || targets[i].y >= SCREEN_HEIGHT / 2) {
                targets[i].speedY = -targets[i].speedY;
            }
        }
    }
}

int checkCollision(Bullet* bullet, Target* target) {
    int dx = bullet->x - target->x;
    int dy = bullet->y - target->y;
    int distance = dx * dx + dy * dy;
    return distance <= (target->radius * target->radius);
}

void handleCollisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_TARGETS; j++) {
                if (!targets[j].hit && checkCollision(&bullets[i], &targets[j])) {
                    targets[j].hit = 1;
                    bullets[i].active = 0;
                    break;
                }
            }
        }
    }
}

void renderPlayer() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for player
    SDL_Rect rect = { player.x, player.y, player.width, player.height };
    SDL_RenderFillRect(renderer, &rect);

    // Represent gun as a small rectangle above the player
    SDL_Rect gun = { player.x + player.width / 2 - 3, player.y - 10, 6, 10 };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for gun
    SDL_RenderFillRect(renderer, &gun);
}

void renderBullets() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect rect = { bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void renderTargets() {
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (!targets[i].hit) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for active targets
        }
        else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray for hit targets
        }
        for (int w = 0; w < targets[i].radius * 2; w++) {
            for (int h = 0; h < targets[i].radius * 2; h++) {
                int dx = targets[i].radius - w;
                int dy = targets[i].radius - h;
                if ((dx * dx + dy * dy) <= (targets[i].radius * targets[i].radius)) {
                    SDL_RenderDrawPoint(renderer, targets[i].x + dx, targets[i].y + dy);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Man with a Gun - Shooting Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    initPlayer();
    initBullets();
    generateTargets();

    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    shootBullet();
                }
                else {
                    movePlayer(&e);
                }
            }
        }

        moveBullets();
        moveTargets();
        handleCollisions();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        renderPlayer();
        renderBullets();
        renderTargets();

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
return 0;
}

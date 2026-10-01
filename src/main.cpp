#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Player {
    float x;
    float y;
    float speed = 200.0f; // pixels per second

    void update(float deltaTime, const bool* keys) {
        if (keys[SDL_SCANCODE_W]) {
            y -= speed * deltaTime;
        }
        if (keys[SDL_SCANCODE_A]) {
            x -= speed * deltaTime;
        }
        if (keys[SDL_SCANCODE_S]) {
            y += speed * deltaTime;
        }
        if (keys[SDL_SCANCODE_D]) {
            x += speed * deltaTime;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_FRect rect{x, y, 100.0f, 100.0f};
        SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255); // orange
        SDL_RenderFillRect(renderer, &rect);
    }
};

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Hello SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Window/renderer creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(renderer, true);

    bool running = true;
    int64_t now = SDL_GetTicksNS();
    int64_t lastTime = now;
    float deltaTime = 0.0;
    Player player{350.0f, 250.0f};
    while (running) {
        lastTime = now;
        now = SDL_GetTicksNS();
        deltaTime = (now - lastTime) / 1000000000.0; // convert nanoseconds to seconds
        // 1. Handle input/events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // 2. Draw
        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);   // background
        SDL_RenderClear(renderer);

        const bool* keys = SDL_GetKeyboardState(nullptr);
        player.update(deltaTime, keys);
        player.render(renderer);

        // 3. Show the frame
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

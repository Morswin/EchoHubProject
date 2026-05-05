#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("EchoHub", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  // Exits the app
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

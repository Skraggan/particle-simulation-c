#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "../include/constants.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int game_is_running = FALSE;

struct timespec last_frame_time;
int current_fps = 0;

struct particle
{
    float x;
    float y;
    float width;
    float height;
} particle;

int initialize(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != FALSE)
    {
        fprintf(stderr, "Error initializing SDL. \n");
        return FALSE;
    }

    window = SDL_CreateWindow(
        "Particle Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window)
    {
        fprintf(stderr, "Error creating SDL Window. \n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL Renderer. \n");
        return FALSE;
    }

    if (!(timespec_get(&last_frame_time, TIME_UTC) == TIME_UTC))
    {
        fprintf(stderr, "Error initializing Timespec Clock. \n");
        return FALSE;
    }

    return TRUE;
}

int setup()
{
    particle.x = 200;
    particle.y = 200;
    particle.width = 100;
    particle.height = 100;
}

void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        game_is_running = FALSE;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            game_is_running = FALSE;
        break;
    }
}

int update()
{
    struct timespec current_time;
    timespec_get(&current_time, TIME_UTC);

    long double delta_time = (current_time.tv_sec - last_frame_time.tv_sec) + ((current_time.tv_nsec - last_frame_time.tv_nsec) / 1000000000.0);
    current_fps = (int)1 / delta_time;

    last_frame_time = current_time;

    particle.x += 70 * delta_time;
    particle.y += 30 * delta_time;
}

int render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect particle_rect = {
        (int)particle.x,
        (int)particle.y,
        (int)particle.width,
        (int)particle.height};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &particle_rect);

    SDL_RenderPresent(renderer);
}

void quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    game_is_running = initialize();

    setup();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    quit();

    return 0;
}
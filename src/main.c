#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "../include/constants.h"
#include "../include/SDL_FontCache.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int game_is_running = FALSE;

struct timespec last_frame_time;
int current_fps = 0;

FC_Font *font = NULL;

struct particle
{
    double position[2];
    double velocity[2];
    double radius;
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

    if (TTF_Init() < 0)
    {
        fprintf(stderr, "Error initializing SDL TTF: %s\n", TTF_GetError());
        return FALSE;
    }

    return TRUE;
}

void setup()
{
    font = FC_CreateFont();
    FC_LoadFont(font, renderer, "assets/fonts/ARIAL.ttf", 14, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);

    SDL_Surface *icon = IMG_Load("assets/images/icon32.png");
    SDL_SetWindowIcon(window, icon);

    particle.position[0] = WINDOW_WIDTH / 2;
    particle.position[1] = WINDOW_HEIGHT / 2;
    particle.velocity[0] = 500;
    particle.velocity[1] = -1500;
    particle.radius = 100;
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

void drawParticle(struct particle *particle)
{
    // SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color light_red = {255, 70, 70, 255};
    SDL_Color blue = {0, 0, 255, 255};

    const double pi = acos(-1);

    int vertices_len = 3;
    SDL_Vertex vertices[vertices_len];

    // Center vertex
    vertices[0].position.x = particle->position[0];
    vertices[0].position.y = particle->position[1];
    vertices[0].color = red;

    // Initial second vertex (straight right)
    vertices[2].position.x = particle->position[0];
    vertices[2].position.y = particle->position[1] + particle->radius;
    vertices[2].color = blue;

    for (int i = 1; i <= CIRCLE_RESOLUTION; i++)
    {

        // First vertex (equal to last iteration's second vertex)
        vertices[1].position.x = vertices[2].position.x;
        vertices[1].position.y = vertices[2].position.y;
        vertices[1].color = light_red;

        // Second vertex
        vertices[2].position.x = particle->position[0] + (particle->radius * sin(((2 * pi) / CIRCLE_RESOLUTION) * i));
        vertices[2].position.y = particle->position[1] + (particle->radius * cos(((2 * pi) / CIRCLE_RESOLUTION) * i));
        vertices[2].color = light_red;

        // Render Circle
        SDL_RenderGeometry(renderer, 0, vertices, vertices_len, NULL, 0);
    }
}

void handleCollision(struct particle *particle, double *delta_time)
{
    // Top border
    if (particle->position[1] - particle->radius < 0)
    {
        particle->position[1] -= particle->velocity[1] * *delta_time;
        particle->velocity[1] = particle->velocity[1] * ABSORPTION_FACTOR;
        particle->velocity[0] = particle->velocity[0] * ABSORPTION_FACTOR;
        particle->velocity[1] = particle->velocity[1] * -1;
    }

    // Bottom border
    if (particle->position[1] + particle->radius > WINDOW_HEIGHT)
    {
        particle->position[1] -= particle->velocity[1] * *delta_time;
        particle->velocity[1] = particle->velocity[1] * ABSORPTION_FACTOR;
        particle->velocity[0] = particle->velocity[0] * ABSORPTION_FACTOR;
        particle->velocity[1] = particle->velocity[1] * -1;
    }

    // Left border
    if (particle->position[0] - particle->radius < 0)
    {
        particle->position[0] -= particle->velocity[0] * *delta_time;
        particle->velocity[0] = particle->velocity[0] * ABSORPTION_FACTOR;
        particle->velocity[1] = particle->velocity[1] * ABSORPTION_FACTOR;
        particle->velocity[0] = particle->velocity[0] * -1;
    }

    // Right border
    if (particle->position[0] + particle->radius > WINDOW_WIDTH)
    {
        particle->position[0] -= particle->velocity[0] * *delta_time;
        particle->velocity[0] = particle->velocity[0] * ABSORPTION_FACTOR;
        particle->velocity[1] = particle->velocity[1] * ABSORPTION_FACTOR;
        particle->velocity[0] = particle->velocity[0] * -1;
    }
}

void update()
{
    // Update time variables
    struct timespec current_time;
    timespec_get(&current_time, TIME_UTC);
    double delta_time = (current_time.tv_sec - last_frame_time.tv_sec) + ((current_time.tv_nsec - last_frame_time.tv_nsec) / 1000000000.0);
    last_frame_time = current_time;
    current_fps = (int)1 / delta_time;

    // SDL_Delay(200);

    // Update position and velocity
    particle.position[0] += particle.velocity[0] * delta_time;
    particle.position[1] += particle.velocity[1] * delta_time;
    particle.velocity[1] += ACCELERATION * delta_time;

    handleCollision(&particle, &delta_time);

    // printf("delta time: %lf, multiplication:%lf\n", delta_time, 70 * delta_time);
    // printf("X: %lf, Y: %lf\n", particle.position[0], particle.position[1]);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawParticle(&particle);

    // Render FPS counter
    FC_Draw(font, renderer, 0, 0, "FPS: %d", current_fps);

    SDL_RenderPresent(renderer);
}

void quit()
{
    FC_FreeFont(font);
    TTF_Quit();
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
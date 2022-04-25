#include <SDL.h>
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <glm.hpp>
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct Transform
{
    int entity_id = 0;
    glm::vec2 location;
    float rotation;
};

struct Pellet
{
    int entity_id = 0;
    int points;
};

struct Velocity
{
    int entity_id = 0;
    glm::vec2 velocity;
};

struct RenderableShape
{
    int entity_id = 0;
    glm::vec3 color;
    glm::vec2 size;
};

struct PlayerControlled
{
    int entity_id = 0;
    float movement_speed;
    bool moving_up;
    bool moving_down;
    bool moving_left;
    bool moving_right;
};

struct Collider
{
    int entity_id = 0;
    glm::vec2 size;
    int chunk_id = 0;
};

//stores all entiries
struct World 
{
    std::map<int, std::unique_ptr<Transform>> transforms;
    std::map<int, std::unique_ptr<Velocity>> velocities;
    std::map<int, std::unique_ptr<RenderableShape>> shapes;
    std::map<int, std::unique_ptr<PlayerControlled>> players;
    std::map<int, std::unique_ptr<Pellet>> pellets;
    std::map<int, std::unique_ptr<Collider>> colliders;
    int points = 0;
    int last = -1;
};

int add_entity(World* world)
{
    return ++world->last;
}

void remove_entity(World* world, int id)
{
    world->transforms.erase(id);
    world->velocities.erase(id);
}
void update_movement(World* world,Transform* transform, Velocity* vel)
{
    transform->location += vel->velocity;
}

void update_player_movement(World* world,PlayerControlled*player, Velocity* velocity)
{
    if (player)
    {
        if (player->moving_down)
        {
            velocity->velocity.y = player->movement_speed;
        }
        else if (player->moving_up)
        {
            velocity->velocity.y = -player->movement_speed;
        }
        else
        {
            velocity->velocity.y = 0;
        }
        if (player->moving_right)
        {
            velocity->velocity.x = player->movement_speed;
        }
        else if (player->moving_left)
        {
            velocity->velocity.x = -player->movement_speed;
        }
        else
        {
            velocity->velocity.x = 0;
        }
    }
}

void update_collider_pellet(World* world, Collider* collider)
{
    for (auto const& [id, Collider]  : world->colliders)
    {
        if (world->players.contains(id) && id != collider->entity_id && world->transforms.contains(collider->entity_id))
        {
            const float& x = world->transforms[id]->location.x;
            const float& y = world->transforms[id]->location.y;
            const float& w = Collider->size.x;
            const float& h = Collider->size.y;

            const float& src_x = world->transforms[collider->entity_id]->location.x;
            const float& src_y = world->transforms[collider->entity_id]->location.y;
            const float& src_w = collider->size.x;
            const float& src_h = collider->size.y;
            if (std::max(x,src_x) < std::min(x + w,src_x + src_w) &&
                std::max(y, src_y) < std::min(y + h, src_y + src_h))
            {
                //printf("Collided! with %i", collider->entity_id);
                remove_entity(world, collider->entity_id);
                std::cout << "Point count: " << ++world->points << std::endl;
            }
        }
    }
}
        
int make_pellet(World* world, glm::vec2 location, int points)
{
    int id = add_entity(world);
    world->pellets[id] = std::make_unique<Pellet>();
    world->transforms[id] = std::make_unique<Transform>();
    world->shapes[id] = std::make_unique<RenderableShape>();
    world->colliders[id] = std::make_unique<Collider>();
    
    world->pellets[id]->points = points;
    world->transforms[id]->location = location;
    world->shapes[id]->color = glm::vec3(155, 155, 155);
    world->shapes[id]->size = glm::vec2(5, 5);
    world->colliders[id]->size = glm::vec2(5, 5);

    world->pellets[id]->entity_id = id;
    world->transforms[id]->entity_id = id;
    world->shapes[id]->entity_id = id;
    world->colliders[id]->entity_id = id;
    return id;
}


int make_player(World* world, glm::vec2 location)
{
    int id = add_entity(world);
    world->players[id] = std::make_unique<PlayerControlled>();
    world->transforms[id] = std::make_unique<Transform>();
    world->velocities[id] = std::make_unique<Velocity>();
    world->shapes[id] = std::make_unique<RenderableShape>();
    world->colliders[id] = std::make_unique<Collider>();
    
    world->shapes[id]->color = glm::vec3(155, 0, 155);
    world->shapes[id]->size = glm::vec2(25, 25);
    world->players[id]->movement_speed = 0.05f;
    world->colliders[id]->size = glm::vec2(25, 25);

    world->players[id]->entity_id = id;
    world->transforms[id]->entity_id = id;
    world->velocities[id]->entity_id = id;
    world->shapes[id]->entity_id = id;
    world->colliders[id]->entity_id = id;

    return id;
}

int main(int argc, char** args) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        return 1;
    }
    SDL_Surface* screen_surface = NULL;

    SDL_Window* window = NULL;
    window = SDL_CreateWindow("Hello, SDL 2!", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (window == NULL) {
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    screen_surface = SDL_GetWindowSurface(window);

    std::unique_ptr<World> world = std::make_unique<World>();
    make_pellet(world.get(), glm::vec2(100, 0), 1);
    make_pellet(world.get(), glm::vec2(100, 100), 1);
    make_pellet(world.get(), glm::vec2(100, 200), 1);
    make_pellet(world.get(), glm::vec2(100, 300), 1);

    make_player(world.get(), glm::vec2(0, 0));
    int frames = 10;
    while (true)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);
        SDL_Event event;
        /* Poll for events. SDL_PollEvent() returns 0 when there are no  */
        /* more events on the event queue, our while loop will exit when */
        /* that occurs.                                                  */
        while (SDL_PollEvent(&event)) {
            /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
            switch (event.type) {
            case SDL_KEYDOWN:
                for (int i = 0; i <= world->last; i++)
                {
                    if (world->players.contains(i))
                    {
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_UP:
                            world->players[i]->moving_up = true;
                            break;
                        case SDLK_DOWN:
                            world->players[i]->moving_down = true;
                            break;
                        case SDLK_RIGHT:
                            world->players[i]->moving_right = true;
                            break;
                        case SDLK_LEFT:
                            world->players[i]->moving_left = true;
                            break;
                        }
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    exit(1);
                }

                //printf("Key press detected\n");
                break;

            case SDL_KEYUP:
                for (int i = 0; i <= world->last; i++)
                {
                    if (world->players.contains(i))
                    {
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_UP:
                            world->players[i]->moving_up = false;
                            break;
                        case SDLK_DOWN:
                            world->players[i]->moving_down = false;
                            break;
                        case SDLK_RIGHT:
                            world->players[i]->moving_right = false;
                            break;
                        case SDLK_LEFT:
                            world->players[i]->moving_left = false;
                            break;
                        }
                    }
                }
               // printf("Key release detected\n");
                break;

            default:
                break;
            }
        }
        for (int i = 0; i <= world->last; i++)
        {
            if (world->colliders.contains(i) && world->pellets.contains(i))
            {
                update_collider_pellet(world.get(), world->colliders[i].get());
            }
            if (world->players.contains(i) && world->velocities.contains(i))
            {
                update_player_movement(world.get(), world->players[i].get(), world->velocities[i].get());
            }
            if (world->transforms.contains(i) && world->velocities.contains(i))
            {
                update_movement(world.get(), world->transforms[i].get(), world->velocities[i].get());
            }
            if (world->transforms.contains(i) && world->shapes.contains(i))
            {
                SDL_FRect rect = {
                     .x = world->transforms[i]->location.x,
                 .y = world->transforms[i]->location.y,
                 .w = world->shapes[i]->size.x,.h = world->shapes[i]->size.y};
                SDL_SetRenderDrawColor(renderer, world->shapes[i]->color.r, world->shapes[i]->color.g, world->shapes[i]->color.b, 1);
                SDL_RenderFillRectF(renderer, &rect);
            }

        }
        SDL_RenderPresent(renderer);
    }//SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0, 255, 0));

   // SDL_UpdateWindowSurface(window);





    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
};
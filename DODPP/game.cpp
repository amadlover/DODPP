#include "game.hpp"
#include "graphics.hpp"
#include "types.h"

#include <vector>
#include <iostream>

/*
 * OOP struct
 *
 * struct actor {
 *     vec2 position_inputs;
 *     vec2 direction;
 *     vec2 rotation;
 *     vec2 scale;
 *     float positional_speed;
 *     float rotational_speed;
 *     char name[256];
 *     asset* geometry;
 * }
 *
 * */

size_t game_current_max_actor_count = 0;
size_t game_live_actor_count = 0;
const size_t game_ACTORS_BATCH_SIZE = 50;

std::vector<vec2> game_actors_positions;
std::vector<vec2> game_actors_rotations;
std::vector<position_inputs> game_actors_positions_inputs;

AGE_RESULT game_init (HINSTANCE h_instance, HWND h_wnd)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_current_max_actor_count = game_ACTORS_BATCH_SIZE;

    game_actors_positions.reserve (game_current_max_actor_count);
    game_actors_rotations.reserve (game_current_max_actor_count);
    game_actors_positions_inputs.reserve (game_current_max_actor_count);

    age_result = graphics_common_graphics_init (h_instance, h_wnd);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_create_transforms_buffer ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_init ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_add_actor (size_t x, size_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_live_actor_count == game_current_max_actor_count)
    {
        game_current_max_actor_count += game_ACTORS_BATCH_SIZE;
        
        game_actors_positions.reserve (game_current_max_actor_count);
        game_actors_rotations.reserve (game_current_max_actor_count);
        game_actors_positions_inputs.reserve (game_current_max_actor_count);
    }

    std::srand (std::rand ());

    game_actors_positions.emplace_back (((float)std::rand () / (float)RAND_MAX) * 2 - 1, ((float)std::rand () / (float)RAND_MAX) * 2 - 1);
    game_actors_rotations.emplace_back (((float)std::rand () / (float)RAND_MAX) * 360.f, ((float)std::rand () / (float)RAND_MAX) * 10.f);

    ++game_live_actor_count;

    std::cout << "current max actors " << game_current_max_actor_count << " actor count " << game_live_actor_count << " ACTOR BATCH SIZE " << game_ACTORS_BATCH_SIZE << "\n";

exit:
    return age_result;
}

AGE_RESULT game_process_left_mouse_click (const size_t x, const size_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = game_add_actor (x, y);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_update_command_buffers ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_update (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = graphics_update_transforms_buffer ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_submit_present (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = graphics_submit_present();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

void game_exit (void)
{
    graphics_exit ();
}

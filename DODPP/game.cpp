#include "game.hpp"
#include "vulkan_graphics.hpp"
#include "types.hpp"
#include "vulkan_interface.hpp"

#include "utils.hpp"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>


bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_d_pressed = false;
bool is_a_pressed = false;
bool is_space_bar_pressed = false;
bool is_up_arrow_pressed = false;
bool is_down_arrow_pressed = false;
bool is_right_arrow_pressed = false;
bool is_left_arrow_pressed = false;

player_transform_inputs game_player_transform_inputs = {};
float2 game_player_output_position;
float game_player_output_rotation;
float2 game_player_output_scale;

float game_player_shooting_interval_msecs = 100.f;
float game_secs_since_last_shot = 0;

std::vector<bullet_transform_inputs> game_bullets_transform_inputs;
std::vector<size_t> game_bullets_current_lifetimes_msecs;

std::vector<float2> game_bullets_outputs_positions;
std::vector<float> game_bullets_outputs_rotations;
std::vector<float2> game_bullets_outputs_scales;

float game_bullets_max_lifetime_msecs = 500.f;

std::vector<asteroid_transform_inputs> game_asteroids_transform_inputs;
std::vector<float2> game_asteroids_outputs_positions;
std::vector<float> game_asteroids_outputs_rotations;
std::vector<float2> game_asteroids_outputs_scales;

size_t game_asteroids_current_max_count = 0;
size_t game_asteroid_live_count = 0;
size_t game_bullets_current_max_count = 0;
size_t game_bullet_live_count = 0;
const size_t game_ASTEROID_BATCH_SIZE = 20;
const size_t game_BULLET_BATCH_SIZE = 20;

RECT window_rect;
int32_t last_mouse_x;
int32_t last_mouse_y;

size_t game_delta_time = 0;

AGE_RESULT game_reserve_memory_for_asteroids_bullets (void);

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_player_transform_inputs.time_msecs_to_come_to_rest = 500.f;
    game_player_transform_inputs.forward_vector.x = 0;
    game_player_transform_inputs.forward_vector.y = 1;
    game_player_transform_inputs.acceleration = 0.000125f;
    game_player_transform_inputs.deceleration = -0.0000625f;
    game_player_transform_inputs.rotation_speed = 0.005f;
    game_player_transform_inputs.max_velocity = 0.05f;

    GetClientRect (h_wnd, &window_rect);

    srand (time_t (NULL));

    age_result = game_reserve_memory_for_asteroids_bullets ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = vulkan_interface_init (h_instance, h_wnd);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_init (game_asteroids_current_max_count, game_asteroid_live_count, game_bullets_current_max_count, game_bullet_live_count);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:  // place to clean up local allocations
    return age_result;
}

AGE_RESULT game_reserve_memory_for_asteroids_bullets ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_asteroids_current_max_count += game_ASTEROID_BATCH_SIZE;
    game_asteroids_transform_inputs.reserve (game_asteroids_current_max_count);

    game_asteroids_outputs_positions.reserve (game_asteroids_current_max_count);
    game_asteroids_outputs_rotations.reserve (game_asteroids_current_max_count);
    game_asteroids_outputs_scales.reserve (game_asteroids_current_max_count);

    game_bullets_current_max_count += game_BULLET_BATCH_SIZE;
    game_bullets_transform_inputs.reserve (game_bullets_current_max_count);
    game_bullets_current_lifetimes_msecs.reserve (game_bullets_current_max_count);

    game_bullets_outputs_positions.reserve (game_bullets_current_max_count);
    game_bullets_outputs_rotations.reserve (game_bullets_current_max_count);
    game_bullets_outputs_scales.reserve (game_bullets_current_max_count);

exit: // clean up allocations done in this function

    return age_result;
}

AGE_RESULT game_asteroid_add (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_asteroid_live_count == game_asteroids_current_max_count)
    {
        game_asteroids_current_max_count += game_ASTEROID_BATCH_SIZE;

        game_asteroids_transform_inputs.reserve (game_asteroids_current_max_count);

        game_asteroids_outputs_positions.reserve (game_asteroids_current_max_count);
        game_asteroids_outputs_rotations.reserve (game_asteroids_current_max_count);
        game_asteroids_outputs_scales.reserve (game_asteroids_current_max_count);
        
        age_result = graphics_create_transforms_buffer (game_asteroids_current_max_count + game_bullets_current_max_count);
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    srand (rand ());

    game_asteroids_outputs_positions.emplace_back (float2 (((float)rand () / (float)RAND_MAX) * 2 - 1, ((float)rand () / (float)RAND_MAX) * 2 - 1));
    game_asteroids_outputs_rotations.emplace_back ((float)rand () / (float)RAND_MAX * 3.14f);
    game_asteroids_outputs_scales.emplace_back (float2 (1, 1));

    game_asteroids_transform_inputs.emplace_back (
        float2 (((float)rand () / (float)RAND_MAX) * 2 - 1, ((float)rand () / (float)RAND_MAX) * 2 - 1), 
        ((float)rand () / (float)RAND_MAX) / 1000.f, 
        (((float)rand () / (float)RAND_MAX)) / 100.f
    );

    ++game_asteroid_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;;
}

AGE_RESULT game_process_left_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = game_asteroid_add ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_asteroid_remove (const size_t& index_to_remove)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_asteroid_live_count == 0)
    {
        goto exit;
    }

    game_asteroids_transform_inputs.erase (std::begin (game_asteroids_transform_inputs) + index_to_remove);

    game_asteroids_outputs_positions.erase (std::begin (game_asteroids_outputs_positions) + index_to_remove);
    game_asteroids_outputs_rotations.erase (std::begin (game_asteroids_outputs_rotations) + index_to_remove);
    game_asteroids_outputs_scales.erase (std::begin (game_asteroids_outputs_scales) + index_to_remove);

    --game_asteroid_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_process_right_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = game_asteroid_remove ((size_t)(((float)rand () / (float)RAND_MAX) * game_asteroid_live_count));
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_update_player_vectors (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    float new_vector_x = -sinf (game_player_transform_inputs.rotation);
    float new_vector_y = cosf (game_player_transform_inputs.rotation);

    game_player_transform_inputs.forward_vector.x = new_vector_x;
    game_player_transform_inputs.forward_vector.y = new_vector_y;

exit:
    return age_result;
}

AGE_RESULT game_process_mouse_move (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (x < 0 || y < 0)
    {
        goto exit;
    }

    last_mouse_x = x;
    last_mouse_y = y;

exit:
    return age_result;
}

AGE_RESULT game_player_increase_speed (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        goto exit;
    }

    float2 acceleration = { 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.x, 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (acceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (acceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

exit:
    return age_result;
}

AGE_RESULT game_player_decrease_speed (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        goto exit;
    }

    float2 deceleration = { 
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.x,
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (deceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (deceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

exit:
    return age_result;
}

AGE_RESULT game_player_turn_right (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_player_transform_inputs.rotation -= (game_player_transform_inputs.rotation_speed * game_delta_time);
    game_player_output_rotation = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_player_turn_left (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;
    
    game_player_transform_inputs.rotation += (game_player_transform_inputs.rotation_speed * game_delta_time);
    game_player_output_rotation = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }
exit:
    return age_result;
}

AGE_RESULT game_bullet_add (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_bullet_live_count == game_bullets_current_max_count)
    {
        game_bullets_current_max_count += game_BULLET_BATCH_SIZE;
        game_bullets_transform_inputs.reserve (game_bullets_current_max_count);
        game_bullets_current_lifetimes_msecs.reserve (game_bullets_current_max_count);

        game_bullets_outputs_positions.reserve (game_bullets_current_max_count);
        game_bullets_outputs_rotations.reserve (game_bullets_current_max_count);
        game_bullets_outputs_scales.reserve (game_bullets_current_max_count);

        age_result = graphics_create_transforms_buffer (game_asteroids_current_max_count + game_bullets_current_max_count);
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    srand (rand ());

    game_bullets_transform_inputs.emplace_back (
        bullet_transform_inputs (game_player_transform_inputs.forward_vector, (float2_length (&game_player_transform_inputs.v) / (float)game_delta_time) + 0.005f)
    );
    game_bullets_current_lifetimes_msecs.emplace_back (0);

    game_bullets_outputs_positions.emplace_back (game_player_output_position);
    game_bullets_outputs_rotations.emplace_back (game_player_output_rotation);
    game_bullets_outputs_scales.emplace_back (float2 (1, 1));

    ++game_bullet_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_bullet_remove (const size_t& index_to_remove)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_bullet_live_count == 0)
    {
        goto exit;
    }

    game_bullets_transform_inputs.erase (std::begin (game_bullets_transform_inputs) + index_to_remove);
    game_bullets_current_lifetimes_msecs.erase (std::begin (game_bullets_current_lifetimes_msecs) + index_to_remove);

    game_bullets_outputs_positions.erase (std::begin (game_bullets_outputs_positions) + index_to_remove);
    game_bullets_outputs_rotations.erase (std::begin (game_bullets_outputs_rotations) + index_to_remove);
    game_bullets_outputs_scales.erase (std::begin (game_bullets_outputs_scales) + index_to_remove);

    --game_bullet_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_player_attempt_to_shoot (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_secs_since_last_shot > game_player_shooting_interval_msecs)
    {
        age_result = game_bullet_add ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }

        game_secs_since_last_shot = 0;
    }

exit:
    return age_result;
}

/*AGE_RESULT game_bullets_check_life (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        game_bullets_current_lifetimes_msecs[b] += game_delta_time;
    }

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        if (game_bullets_current_lifetimes_msecs[b] > game_bullets_max_lifetime_msecs)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                goto exit;
            }
        }
    }

exit:
    return age_result;
}*/

AGE_RESULT game_process_key_down (const WPARAM w_param)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;
    
    switch (w_param) 
    {
        case 0x57: // w
        is_w_pressed = true;
        break;

        case 0x53: // s
        is_s_pressed = true;
        break;

        case 0x44: // d
        is_d_pressed = true;
        break;
        
        case 0x41: // a
        is_a_pressed = true;
        break;

        case 0x26: // up arrow
        is_up_arrow_pressed = true;
        break;

        case 0x28: // down arrow
        is_down_arrow_pressed = true;
        break;

        case 0x27: // right arrow
        is_right_arrow_pressed = true;
        break;

        case 0x25: // left arrow
        is_left_arrow_pressed = true;
        break;

        case 0x20: // space
        is_space_bar_pressed = true;
        break;

        default:
        break;
    }

exit:
    return age_result;
}

AGE_RESULT game_process_key_up (const WPARAM w_param)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    switch (w_param) 
    {
        case 0x57: // w
        is_w_pressed = false;
        break;

        case 0x53: // s
        is_s_pressed = false;
        break;

        case 0x44: // d
        is_d_pressed = false;
        break;
        
        case 0x41: // a
        is_a_pressed = false;
        break;

        case 0x26: // up arrow
        is_up_arrow_pressed = false;
        break;

        case 0x28: // down arrow
        is_down_arrow_pressed = false;
        break;

        case 0x27: // right arrow
        is_right_arrow_pressed = false;
        break;

        case 0x25: // left arrow
        is_left_arrow_pressed = false;
        break;

        case 0x20: // space
        is_space_bar_pressed = false;
        break;

        default:
        break;
    }

exit:
    return age_result;
}

AGE_RESULT game_update_player_asteroids_bullets_output_positions (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_player_output_position.x += game_player_transform_inputs.v.x;
    game_player_output_position.y += game_player_transform_inputs.v.y;

    if (game_player_output_position.x > 1.f)
    {
        game_player_output_position.x = -1.f;
    }
    
    if (game_player_output_position.x < -1.f)
    {
        game_player_output_position.x = 1.f;
    }

    if (game_player_output_position.y > 1.f)
    {
        game_player_output_position.y = -1.f;
    }
    
    if (game_player_output_position.y < -1.f)
    {
        game_player_output_position.y = 1.f;
    }

    for (size_t a = 0; a < game_asteroid_live_count; ++a)
    {
        game_asteroids_outputs_positions[a].x += (game_asteroids_transform_inputs[a].forward_vector.x * game_asteroids_transform_inputs[a].forward_speed * game_delta_time);
        game_asteroids_outputs_positions[a].y += (game_asteroids_transform_inputs[a].forward_vector.y * game_asteroids_transform_inputs[a].forward_speed * game_delta_time);

        if (game_asteroids_outputs_positions[a].x > 1.f)
        {
            game_asteroids_outputs_positions[a].x = -1.f;
        }
        else if (game_asteroids_outputs_positions[a].x < -1.f)
        {
            game_asteroids_outputs_positions[a].x = 1.f;
        }
        else if (game_asteroids_outputs_positions[a].y > 1.f)
        {
            game_asteroids_outputs_positions[a].y = -1.f;
        }
        else if (game_asteroids_outputs_positions[a].y < -1.f)
        {
            game_asteroids_outputs_positions[a].y = 1.f;
        }

        game_asteroids_outputs_rotations[a] += (game_asteroids_transform_inputs[a].rotation_speed) * game_delta_time;
    }

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        game_bullets_outputs_positions[b].x += (game_bullets_transform_inputs[b].forward_vector.x * (game_bullets_transform_inputs[b].speed * game_delta_time));
        game_bullets_outputs_positions[b].y += (game_bullets_transform_inputs[b].forward_vector.y * (game_bullets_transform_inputs[b].speed * game_delta_time));

        if (game_bullets_outputs_positions[b].x < -1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                goto exit;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].x > 1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                goto exit;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].y < -1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                goto exit;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].y > 1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                goto exit;
            }

            break;
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_player_apply_damping (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    float damping_value = 1;

    if (game_delta_time < game_player_transform_inputs.time_msecs_to_come_to_rest)
    {
        damping_value = (1 - (game_delta_time / game_player_transform_inputs.time_msecs_to_come_to_rest));
    }

    game_player_transform_inputs.v.x *= damping_value;
    game_player_transform_inputs.v.y *= damping_value;

    game_player_transform_inputs.u.x *= damping_value;
    game_player_transform_inputs.u.y *= damping_value;

exit:
    return age_result;
}

AGE_RESULT game_process_player_input (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (is_w_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_up_arrow_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }
    
    if (is_s_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_down_arrow_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_d_pressed)
    {
        age_result = game_player_turn_right ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_right_arrow_pressed)
    {
        age_result = game_player_turn_right ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_a_pressed)
    {
        age_result = game_player_turn_left ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_left_arrow_pressed)
    {
        age_result = game_player_turn_left ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

    if (is_space_bar_pressed)
    {
        age_result = game_player_attempt_to_shoot ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            goto exit;
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_bullets_asteroids_collision_checks (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        for (size_t a = 0; a < game_asteroid_live_count; ++a)
        {
            float2 diff = {
                game_asteroids_outputs_positions[a].x - game_bullets_outputs_positions[b].x,
                game_asteroids_outputs_positions[a].y - game_bullets_outputs_positions[b].y,
            };

            if (hypotf (diff.x, diff.y) < 0.1f)
            {
                age_result = game_bullet_remove (b);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    goto exit;
                }

                age_result = game_asteroid_remove (a);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    goto exit;
                }

                break;
            }
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_update (size_t delta_msecs)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_delta_time = delta_msecs;

    game_secs_since_last_shot += game_delta_time;;

    age_result = game_process_player_input ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = game_update_player_asteroids_bullets_output_positions ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = game_player_apply_damping ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    /*age_result = game_bullets_check_life ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }*/

    age_result = game_bullets_asteroids_collision_checks ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_update_transforms_buffer_data (
        &game_player_output_position,
        &game_player_output_rotation,
        &game_player_output_scale,
        game_asteroids_outputs_positions.data (),
        game_asteroids_outputs_rotations.data (),
        game_asteroids_outputs_scales.data(),
        game_asteroid_live_count,
        game_asteroids_current_max_count,
        game_bullets_outputs_positions.data (),
        game_bullets_outputs_rotations.data (),
        game_bullets_outputs_scales.data (),
        game_bullet_live_count,
        game_bullets_current_max_count
    );
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit: // clear function specific allocations
    return age_result;
}

AGE_RESULT game_submit_present (void)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = graphics_submit_present ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

void game_shutdown (void)
{
    graphics_shutdown ();
    vulkan_interface_shutdown ();
    
    game_asteroids_transform_inputs.clear ();

    game_asteroids_outputs_positions.clear ();
    game_asteroids_outputs_rotations.clear ();
    game_asteroids_outputs_scales.clear ();

    game_bullets_transform_inputs.clear ();
    game_bullets_current_lifetimes_msecs.clear ();

    game_bullets_outputs_positions.clear ();
    game_bullets_outputs_rotations.clear ();
    game_bullets_outputs_scales.clear ();
}
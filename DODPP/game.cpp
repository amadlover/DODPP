#include "game.hpp"
#include "graphics.hpp"
#include "types.hpp"
#include "vulkan_interface.hpp"

#include "utils.hpp"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>

bool use_mouse_for_look_at = false;
bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_d_pressed = false;
bool is_a_pressed = false;
bool is_space_bar_pressed = false;
bool is_up_arrow_pressed = false;
bool is_down_arrow_pressed = false;
bool is_right_arrow_pressed = false;
bool is_left_arrow_pressed = false;

float screen_aspect_ratio = 1;

player_transform_inputs game_player_transform_inputs = {};
float2 game_player_output_position;
float2 game_player_output_rotation;
float2 game_player_output_scale;

float game_player_shooting_interval_msecs = 100.f;
float game_secs_since_last_shot = 0;

bullet_transform_inputs* game_bullets_transform_inputs;

float2* game_bullets_outputs_positions;
float2* game_bullets_outputs_rotations;
float2* game_bullets_outputs_scales;

float game_bullets_max_lifetime_msecs = 500.f;

asteroid_transform_inputs* game_large_asteroids_transform_inputs;
float2* game_large_asteroids_outputs_positions;
float2* game_large_asteroids_outputs_rotations;
float2* game_large_asteroids_outputs_scales;

asteroid_transform_inputs* game_small_asteroids_transform_inputs;
float2* game_small_asteroids_outputs_positions;
float2* game_small_asteroids_outputs_rotations;
float2* game_small_asteroids_outputs_scales;

size_t game_large_asteroids_current_max_count = 0;
size_t game_large_asteroids_live_count = 0;

size_t game_small_asteroids_current_max_count = 0;
size_t game_small_asteroids_live_count = 0;

size_t game_bullets_current_max_count = 0;
size_t game_bullet_live_count = 0;

const size_t game_LARGE_ASTEROID_BATCH_SIZE = 50;
const size_t game_SMALL_ASTEROID_BATCH_SIZE = 150;
const size_t game_BULLET_BATCH_SIZE = 20;

RECT scene_rect;
int32_t last_mouse_x;
int32_t last_mouse_y;

size_t game_delta_time = 0;

AGE_RESULT game_reserve_memory_for_asteroids_bullets ();

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    GetClientRect (h_wnd, &scene_rect);
    screen_aspect_ratio = (float)scene_rect.right / (float)scene_rect.bottom;

    game_player_transform_inputs.time_msecs_to_come_to_rest = 500.f;
    game_player_transform_inputs.forward_vector.x = 0;
    game_player_transform_inputs.forward_vector.y = 1;
    game_player_transform_inputs.acceleration = 0.00005f;
    game_player_transform_inputs.deceleration = -0.000025f;
    game_player_transform_inputs.rotation_speed = 0.005f;
    game_player_transform_inputs.max_velocity = 0.05f;
    game_player_output_scale = float2 (1, 1);

    srand (time_t (NULL));

    age_result = game_reserve_memory_for_asteroids_bullets ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    age_result = vulkan_interface_init (h_instance, h_wnd);
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    age_result = graphics_init (
        game_large_asteroids_current_max_count, game_large_asteroids_live_count, 
        game_small_asteroids_current_max_count, game_small_asteroids_live_count, 
        game_bullets_current_max_count, game_bullet_live_count
    );

    return age_result;
}

AGE_RESULT game_reserve_memory_for_asteroids_bullets ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_large_asteroids_current_max_count += game_LARGE_ASTEROID_BATCH_SIZE;
    game_large_asteroids_transform_inputs = (asteroid_transform_inputs*)utils_malloc (sizeof (asteroid_transform_inputs) * game_large_asteroids_current_max_count);

    game_large_asteroids_outputs_positions = (float2*)utils_malloc (sizeof (float2) * game_large_asteroids_current_max_count);
    game_large_asteroids_outputs_rotations = (float2*)utils_malloc (sizeof (float2) * game_large_asteroids_current_max_count);
    game_large_asteroids_outputs_scales = (float2*)utils_malloc (sizeof (float2) * game_large_asteroids_current_max_count);

    game_small_asteroids_current_max_count += game_SMALL_ASTEROID_BATCH_SIZE;
    game_small_asteroids_transform_inputs = (asteroid_transform_inputs*)utils_malloc (sizeof (asteroid_transform_inputs) * game_small_asteroids_current_max_count);

    game_small_asteroids_outputs_positions = (float2*)utils_malloc (sizeof (float2) * game_small_asteroids_current_max_count);
    game_small_asteroids_outputs_rotations = (float2*)utils_malloc (sizeof (float2) * game_small_asteroids_current_max_count);
    game_small_asteroids_outputs_scales = (float2*)utils_malloc (sizeof (float2) * game_small_asteroids_current_max_count);

    game_bullets_current_max_count += game_BULLET_BATCH_SIZE;
    game_bullets_transform_inputs = (bullet_transform_inputs*)utils_malloc (sizeof (bullet_transform_inputs) * game_bullets_current_max_count);

    game_bullets_outputs_positions = (float2*)utils_malloc (sizeof (float2) * game_bullets_current_max_count);
    game_bullets_outputs_rotations = (float2*)utils_malloc (sizeof (float2) * game_bullets_current_max_count);
    game_bullets_outputs_scales = (float2*)utils_malloc (sizeof (float2) * game_bullets_current_max_count);

    return age_result;
}

AGE_RESULT game_large_asteroid_add (float2 position)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_large_asteroids_live_count == game_large_asteroids_current_max_count)
    {
        game_large_asteroids_current_max_count += game_LARGE_ASTEROID_BATCH_SIZE;

        game_large_asteroids_transform_inputs = (asteroid_transform_inputs*)utils_realloc (game_large_asteroids_transform_inputs, sizeof (asteroid_transform_inputs) * game_large_asteroids_current_max_count);

        game_large_asteroids_outputs_positions = (float2*)utils_realloc (game_large_asteroids_outputs_positions, sizeof (float2) * game_large_asteroids_current_max_count);
        game_large_asteroids_outputs_rotations = (float2*)utils_realloc (game_large_asteroids_outputs_rotations, sizeof (float2) * game_large_asteroids_current_max_count);
        game_large_asteroids_outputs_scales = (float2*)utils_realloc (game_large_asteroids_outputs_scales, sizeof (float2) * game_large_asteroids_current_max_count);
        
        age_result = graphics_create_transforms_buffer (
            game_large_asteroids_current_max_count, 
            game_small_asteroids_current_max_count, 
            game_bullets_current_max_count
        );
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    srand (rand ());
    
    game_large_asteroids_outputs_positions[game_large_asteroids_live_count] = position;
    game_large_asteroids_outputs_rotations[game_large_asteroids_live_count] = float2 ((float)rand () / (float)RAND_MAX * 3.14f, 0);
    game_large_asteroids_outputs_scales[game_large_asteroids_live_count] = float2 (1, 1);

    game_large_asteroids_transform_inputs[game_large_asteroids_live_count].forward_vector = float2 (((float)rand () / (float)RAND_MAX) * 2 - 1, ((float)rand () / (float)RAND_MAX) * 2 - 1);
    game_large_asteroids_transform_inputs[game_large_asteroids_live_count].forward_speed = ((float)rand () / (float)RAND_MAX) / 1000.f;
    game_large_asteroids_transform_inputs[game_large_asteroids_live_count].rotation_speed = (((float)rand () / (float)RAND_MAX)) / 100.f;

    ++game_large_asteroids_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);

    return age_result;
}

AGE_RESULT game_small_asteroid_add (float2 position)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_small_asteroids_live_count == game_small_asteroids_current_max_count)
    {
        game_small_asteroids_current_max_count += game_SMALL_ASTEROID_BATCH_SIZE;

        game_small_asteroids_transform_inputs = (asteroid_transform_inputs*)utils_realloc (game_small_asteroids_transform_inputs, sizeof (asteroid_transform_inputs) * game_small_asteroids_current_max_count);

        game_small_asteroids_outputs_positions = (float2*)utils_realloc (game_small_asteroids_outputs_positions, sizeof (float2) * game_small_asteroids_current_max_count);
        game_small_asteroids_outputs_rotations = (float2*)utils_realloc (game_small_asteroids_outputs_rotations, sizeof (float2) * game_small_asteroids_current_max_count);
        game_small_asteroids_outputs_scales = (float2*)utils_realloc (game_small_asteroids_outputs_scales, sizeof (float2) * game_small_asteroids_current_max_count);

        age_result = graphics_create_transforms_buffer (
            game_large_asteroids_current_max_count, 
            game_small_asteroids_current_max_count, 
            game_bullets_current_max_count
        );
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    srand (rand ());

    game_small_asteroids_outputs_positions[game_small_asteroids_live_count] = position;
    game_small_asteroids_outputs_rotations[game_small_asteroids_live_count] = float2 ((float)rand () / (float)RAND_MAX * 3.14f, 0);
    game_small_asteroids_outputs_scales[game_small_asteroids_live_count] = float2 (0.5, 0.5);

    game_small_asteroids_transform_inputs[game_small_asteroids_live_count].forward_vector = float2 (((float)rand () / (float)RAND_MAX) * 2 - 1, ((float)rand () / (float)RAND_MAX) * 2 - 1);
    game_small_asteroids_transform_inputs[game_small_asteroids_live_count].forward_speed = ((float)rand () / (float)RAND_MAX) / 4000.f;
    game_small_asteroids_transform_inputs[game_small_asteroids_live_count].rotation_speed = (((float)rand () / (float)RAND_MAX)) / 100.f;

    ++game_small_asteroids_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);
    
    return age_result;;
}

AGE_RESULT game_process_left_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;
    
    age_result = game_large_asteroid_add (float2 (((float)rand () / (float)RAND_MAX) * 2 - 1, ((float)rand () / (float)RAND_MAX) * 2 - 1));
    
    return age_result;
}

AGE_RESULT game_large_asteroid_remove (const size_t& index_to_remove)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_large_asteroids_live_count == 0)
    {
        return age_result;
    }

    for (size_t a = index_to_remove; a < game_large_asteroids_live_count; ++a)
    {
        game_large_asteroids_transform_inputs[a] = game_large_asteroids_transform_inputs[a + 1];
    }

    for (size_t a = index_to_remove; a < game_large_asteroids_live_count; ++a)
    {
        game_large_asteroids_outputs_positions[a] = game_large_asteroids_outputs_positions[a + 1];
    }

    for (size_t a = index_to_remove; a < game_large_asteroids_live_count; ++a)
    {
        game_large_asteroids_outputs_rotations[a] = game_large_asteroids_outputs_rotations[a + 1];
    }

    for (size_t a = index_to_remove; a < game_large_asteroids_live_count; ++a)
    {
        game_large_asteroids_outputs_scales[a] = game_large_asteroids_outputs_scales[a + 1];
    }

    --game_large_asteroids_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);

    return age_result;
}

AGE_RESULT game_small_asteroid_remove (const size_t& index_to_remove)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_small_asteroids_live_count == 0)
    {
        return age_result;
    }

    for (size_t a = index_to_remove; a < game_small_asteroids_live_count; ++a)
    {
        game_small_asteroids_transform_inputs[a] = game_small_asteroids_transform_inputs[a + 1];
    }

    for (size_t a = index_to_remove; a < game_small_asteroids_live_count; ++a)
    {
        game_small_asteroids_outputs_positions[a] = game_small_asteroids_outputs_positions[a + 1];
    }

    for (size_t a = index_to_remove; a < game_small_asteroids_live_count; ++a)
    {
        game_small_asteroids_outputs_rotations[a] = game_small_asteroids_outputs_rotations[a + 1];
    }

    for (size_t a = index_to_remove; a < game_small_asteroids_live_count; ++a)
    {
        game_small_asteroids_outputs_scales[a] = game_small_asteroids_outputs_scales[a + 1];
    }

    --game_small_asteroids_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);

    return age_result;
}

AGE_RESULT game_process_right_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = game_large_asteroid_remove ((size_t)(((float)rand () / (float)RAND_MAX) * game_large_asteroids_live_count));

    return age_result;
}

AGE_RESULT game_update_player_vectors ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    float new_vector_x = -sinf (game_player_transform_inputs.rotation);
    float new_vector_y = cosf (game_player_transform_inputs.rotation);

    game_player_transform_inputs.forward_vector.x = new_vector_x;
    game_player_transform_inputs.forward_vector.y = new_vector_y;

    return age_result;
}

AGE_RESULT game_process_mouse_move (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (x < 0 || y < 0)
    {
        return age_result;
    }

    last_mouse_x = x;
    last_mouse_y = y;

    return age_result;
}

AGE_RESULT game_player_increase_speed ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        return age_result;
    }

    float2 acceleration = { 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.x, 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (acceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (acceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

    return age_result;
}

AGE_RESULT game_player_decrease_speed ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        return age_result;
    }

    float2 deceleration = { 
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.x,
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (deceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (deceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

    return age_result;
}

AGE_RESULT game_player_turn_right ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    game_player_transform_inputs.rotation -= (game_player_transform_inputs.rotation_speed * game_delta_time);
    game_player_output_rotation.x = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();

    return age_result;
}

AGE_RESULT game_player_turn_left ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;
    
    game_player_transform_inputs.rotation += (game_player_transform_inputs.rotation_speed * game_delta_time);
    game_player_output_rotation.x = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    
    return age_result;
}

AGE_RESULT game_player_look_at_mouse ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    float2 transformed_mouse_pos = float2 (
        (float)last_mouse_x / (float)scene_rect.right * 2 - 1,
        (float)last_mouse_y / (float)scene_rect.bottom * 2 - 1
    );

    float2 look_at_mouse = float2 (transformed_mouse_pos.x - game_player_output_position.x, transformed_mouse_pos.y - game_player_output_position.y);
    float2_unit_vector (&look_at_mouse);
    game_player_transform_inputs.forward_vector = look_at_mouse;

    game_player_output_rotation.x = -asinf (game_player_transform_inputs.forward_vector.x);
    game_player_output_rotation.y = acosf (game_player_transform_inputs.forward_vector.y);

    return age_result;
}

AGE_RESULT game_bullet_add ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_bullet_live_count == game_bullets_current_max_count)
    {
        game_bullets_current_max_count += game_BULLET_BATCH_SIZE;
        game_bullets_transform_inputs = (bullet_transform_inputs*)utils_realloc (game_bullets_transform_inputs, sizeof (bullet_transform_inputs) * game_bullets_current_max_count);

        game_bullets_outputs_positions = (float2*)utils_realloc (game_bullets_outputs_positions, sizeof (float2) * game_bullets_current_max_count);
        game_bullets_outputs_rotations = (float2*)utils_realloc (game_bullets_outputs_rotations, sizeof (float2) * game_bullets_current_max_count);
        game_bullets_outputs_scales = (float2*)utils_realloc (game_bullets_outputs_scales, sizeof (float2) * game_bullets_current_max_count);

        age_result = graphics_create_transforms_buffer (
            game_large_asteroids_current_max_count, 
            game_small_asteroids_current_max_count, 
            game_bullets_current_max_count
        );
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    game_bullets_transform_inputs[game_bullet_live_count].forward_vector = game_player_transform_inputs.forward_vector;
    game_bullets_transform_inputs[game_bullet_live_count].speed = (float2_length (&game_player_transform_inputs.v) / (float)game_delta_time) + 0.005f;

    game_bullets_outputs_positions[game_bullet_live_count] = game_player_output_position;
    game_bullets_outputs_rotations[game_bullet_live_count] = game_player_output_rotation;
    game_bullets_outputs_scales[game_bullet_live_count] = float2 (0.5, 0.5);

    ++game_bullet_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);

    return age_result;
}

AGE_RESULT game_bullet_remove (const size_t& index_to_remove)
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_bullet_live_count == 0)
    {
        return age_result;
    }

    for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
    {
        game_bullets_transform_inputs[b] = game_bullets_transform_inputs[b + 1];
    }
    
    for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
    {
        game_bullets_outputs_positions[b] = game_bullets_outputs_positions[b + 1];
    }

    for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
    {
        game_bullets_outputs_rotations[b] = game_bullets_outputs_rotations[b + 1];
    }

    for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
    {
        game_bullets_outputs_scales[b] = game_bullets_outputs_scales[b + 1];
    }

    --game_bullet_live_count;

    age_result = graphics_update_command_buffers (game_large_asteroids_live_count, game_small_asteroids_live_count, game_bullet_live_count);

    return age_result;
}

AGE_RESULT game_player_attempt_to_shoot ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (game_secs_since_last_shot > game_player_shooting_interval_msecs)
    {
        age_result = game_bullet_add ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }

        game_secs_since_last_shot = 0;
    }

    return age_result;
}

/*AGE_RESULT game_bullets_check_life ()
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

    return age_result;
}

AGE_RESULT game_update_player_asteroids_bullets_output_positions ()
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

    for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
    {
        game_large_asteroids_outputs_positions[a].x += (game_large_asteroids_transform_inputs[a].forward_vector.x * game_large_asteroids_transform_inputs[a].forward_speed * game_delta_time);
        game_large_asteroids_outputs_positions[a].y += (game_large_asteroids_transform_inputs[a].forward_vector.y * game_large_asteroids_transform_inputs[a].forward_speed * game_delta_time);

        if (game_large_asteroids_outputs_positions[a].x > 1.f)
        {
            game_large_asteroids_outputs_positions[a].x = -1.f;
        }
        else if (game_large_asteroids_outputs_positions[a].x < -1.f)
        {
            game_large_asteroids_outputs_positions[a].x = 1.f;
        }
        else if (game_large_asteroids_outputs_positions[a].y > 1.f)
        {
            game_large_asteroids_outputs_positions[a].y = -1.f;
        }
        else if (game_large_asteroids_outputs_positions[a].y < -1.f)
        {
            game_large_asteroids_outputs_positions[a].y = 1.f;
        }

        game_large_asteroids_outputs_rotations[a].x += (game_large_asteroids_transform_inputs[a].rotation_speed) * game_delta_time;
    }

    for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
    {
        game_small_asteroids_outputs_positions[a].x += (game_small_asteroids_transform_inputs[a].forward_vector.x * game_small_asteroids_transform_inputs[a].forward_speed * game_delta_time);
        game_small_asteroids_outputs_positions[a].y += (game_small_asteroids_transform_inputs[a].forward_vector.y * game_small_asteroids_transform_inputs[a].forward_speed * game_delta_time);

        if (game_small_asteroids_outputs_positions[a].x > 1.f)
        {
            game_small_asteroids_outputs_positions[a].x = -1.f;
        }
        else if (game_small_asteroids_outputs_positions[a].x < -1.f)
        {
            game_small_asteroids_outputs_positions[a].x = 1.f;
        }
        else if (game_small_asteroids_outputs_positions[a].y > 1.f)
        {
            game_small_asteroids_outputs_positions[a].y = -1.f;
        }
        else if (game_small_asteroids_outputs_positions[a].y < -1.f)
        {
            game_small_asteroids_outputs_positions[a].y = 1.f;
        }

        game_small_asteroids_outputs_rotations[a].x += (game_small_asteroids_transform_inputs[a].rotation_speed) * game_delta_time;
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
                return age_result;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].x > 1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].y < -1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }

            break;
        }
        else if (game_bullets_outputs_positions[b].y > 1)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }

            break;
        }
    }

    return age_result;
}

AGE_RESULT game_player_apply_damping ()
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

    return age_result;
}

AGE_RESULT game_process_player_input ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    if (is_w_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    if (is_up_arrow_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }
    
    if (is_s_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    if (is_down_arrow_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    if (use_mouse_for_look_at)
    {
        age_result = game_player_look_at_mouse ();
        if (age_result != AGE_RESULT::SUCCESS) {
            return age_result;
        }
    }
    else
    {
        if (is_d_pressed)
        {
            age_result = game_player_turn_right ();
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }
        }

        if (is_right_arrow_pressed)
        {
            age_result = game_player_turn_right ();
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }
        }

        if (is_a_pressed)
        {
            age_result = game_player_turn_left ();
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }
        }

        if (is_left_arrow_pressed)
        {
            age_result = game_player_turn_left ();
            if (age_result != AGE_RESULT::SUCCESS)
            {
                return age_result;
            }
        }
    }

    if (is_space_bar_pressed)
    {
        age_result = game_player_attempt_to_shoot ();
        if (age_result != AGE_RESULT::SUCCESS)
        {
            return age_result;
        }
    }

    return age_result;
}

AGE_RESULT game_bullets_large_asteroids_collision_checks ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        for (size_t a = 0; a < game_large_asteroids_live_count; ++a)
        {
            float2 diff = {
                game_large_asteroids_outputs_positions[a].x - game_bullets_outputs_positions[b].x,
                game_large_asteroids_outputs_positions[a].y - game_bullets_outputs_positions[b].y,
            };

            if (hypotf (diff.x, diff.y) < 0.1f)
            {
                age_result = game_bullet_remove (b);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    return age_result;
                }

                float2 position = game_large_asteroids_outputs_positions[a];
                float2 scale = game_large_asteroids_outputs_scales[a];

                age_result = game_large_asteroid_remove (a);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    return age_result;
                }

                for (size_t a = 0; a < 3; ++a)
                {
                    age_result = game_small_asteroid_add (position);
                    if (age_result != AGE_RESULT::SUCCESS)
                    {
                        return age_result;
                    }
                }

                break;
            }
        }
    }

    return age_result;
}

AGE_RESULT game_bullets_small_asteroids_collision_checks ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        for (size_t a = 0; a < game_small_asteroids_live_count; ++a)
        {
            float2 diff = {
                game_small_asteroids_outputs_positions[a].x - game_bullets_outputs_positions[b].x,
                game_small_asteroids_outputs_positions[a].y - game_bullets_outputs_positions[b].y,
            };

            if (hypotf (diff.x, diff.y) < 0.05f)
            {
                age_result = game_bullet_remove (b);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    return age_result;
                }

                age_result = game_small_asteroid_remove (a);
                if (age_result != AGE_RESULT::SUCCESS)
                {
                    return age_result;
                }

                break;
            }
        }
    }

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
        return age_result;
    }

    age_result = game_update_player_asteroids_bullets_output_positions ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    age_result = game_player_apply_damping ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    /*age_result = game_bullets_check_life ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        goto exit;
    }*/

    age_result = game_bullets_large_asteroids_collision_checks ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    age_result = game_bullets_small_asteroids_collision_checks ();
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    age_result = graphics_update_transforms_buffer_data (
        &game_player_output_position,
        &game_player_output_rotation,
        &game_player_output_scale,
        game_large_asteroids_outputs_positions,
        game_large_asteroids_outputs_rotations,
        game_large_asteroids_outputs_scales,
        game_large_asteroids_live_count,
        game_large_asteroids_current_max_count,
        game_small_asteroids_outputs_positions,
        game_small_asteroids_outputs_rotations,
        game_small_asteroids_outputs_scales,
        game_small_asteroids_live_count,
        game_small_asteroids_current_max_count,
        game_bullets_outputs_positions,
        game_bullets_outputs_rotations,
        game_bullets_outputs_scales,
        game_bullet_live_count,
        game_bullets_current_max_count,
        screen_aspect_ratio
    );
    if (age_result != AGE_RESULT::SUCCESS)
    {
        return age_result;
    }

    return age_result;
}

AGE_RESULT game_submit_present ()
{
    AGE_RESULT age_result = AGE_RESULT::SUCCESS;

    age_result = graphics_submit_present ();

    return age_result;
}

void game_shutdown ()
{
    graphics_shutdown ();
    vulkan_interface_shutdown ();
    
    utils_free (game_large_asteroids_transform_inputs);

    utils_free (game_large_asteroids_outputs_positions);
    utils_free (game_large_asteroids_outputs_rotations);
    utils_free (game_large_asteroids_outputs_scales);

    utils_free (game_small_asteroids_transform_inputs);

    utils_free (game_small_asteroids_outputs_positions);
    utils_free (game_small_asteroids_outputs_rotations);
    utils_free (game_small_asteroids_outputs_scales);

    utils_free (game_bullets_transform_inputs);

    utils_free (game_bullets_outputs_positions);
    utils_free (game_bullets_outputs_rotations);
    utils_free (game_bullets_outputs_scales);
}
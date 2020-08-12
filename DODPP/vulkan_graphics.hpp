#pragma once

#include "error.hpp"
#include "types.hpp"

#include <Windows.h>

AGE_RESULT graphics_init (
    const size_t game_asteroid_current_max_count, 
    const size_t game_asteroid_live_count, 
    const size_t game_bullet_current_max_count, 
    const size_t game_bullet_live_count
);
AGE_RESULT graphics_update_command_buffers (const size_t game_asteroid_live_count, const size_t game_bullet_live_count);
AGE_RESULT graphics_create_transforms_buffer (const size_t game_asteroid_current_max_count);
AGE_RESULT graphics_update_transforms_buffer_data (
    const actor_transform_outputs* game_player_transform_outputs,
    const actor_transform_outputs* game_asteroids_transform_outputs,
    const size_t game_asteroid_live_count,
    const size_t game_asteroid_current_max_count,
    //const actor_transform_outputs* game_bullets_transform_outputs, 
    const float2* game_bullets_outputs_positions, 
    const float* game_bullets_outputs_rotations, 
    const size_t game_bullet_live_count, 
    const size_t game_bullet_current_max_count
);
AGE_RESULT graphics_submit_present (void);
void graphics_shutdown (void);
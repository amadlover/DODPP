#pragma once

#include "error.hpp"
#include "types.hpp"

#include <Windows.h>

AGE_RESULT graphics_init (
    const size_t game_large_asteroids_current_max_count,
	const size_t game_large_asteroids_live_count,
	const size_t game_small_asteroids_current_max_count,
	const size_t game_small_asteroids_live_count,
	const size_t game_bullets_current_max_count,
	const size_t game_bullet_live_count,
	const float screen_aspect_ratio
);
AGE_RESULT graphics_update_command_buffers (
	const size_t game_large_asteroids_live_count,
	const size_t game_small_asteroids_live_count,
	const size_t game_bullet_live_count, 
	const float screen_aspect_ratio
);
AGE_RESULT graphics_create_transforms_buffer (
	const size_t game_large_asteroids_current_max_count, 
	const size_t game_small_asteroids_current_max_count, 
	const size_t game_bullet_current_max_count
);
AGE_RESULT graphics_update_transforms_buffer_data (
    const float2* game_player_output_position, const float2 * game_player_output_rotation, const float2* game_player_output_scale,
	const float2* game_large_asteroids_outputs_positions, const float2 * game_large_asteroids_outputs_rotations, const float2* game_large_asteroids_outputs_scales,
	const size_t game_large_asteroids_live_count, const size_t game_large_asteroids_current_max_count, 
	const float2* game_small_asteroids_outputs_positions, const float2 * game_small_asteroids_outputs_rotations, const float2* game_small_asteroids_outputs_scales,
	const size_t game_small_asteroids_live_count, const size_t game_small_asteroids_current_max_count, 
    const float2* game_bullets_outputs_positions, const float2 * game_bullets_outputs_rotations, const float2* game_bullets_outputs_scales, 
    const size_t game_bullet_live_count, const size_t game_bullets_current_max_count
);
AGE_RESULT graphics_submit_present ();
void graphics_shutdown ();
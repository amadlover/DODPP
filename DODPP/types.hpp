#pragma once

#include <stdint.h>


typedef struct float2_
{
    float x;
    float y;
} float2;

typedef struct float3_
{
    float x;
    float y;
    float z;
} float3;

typedef struct player_transform_inputs_
{
    float acceleration;
    float deceleration;
    float2 v;
    float2 u;
    float2 forward_vector;
    float rotation;
    float rotation_speed;
    float time_msecs_to_come_to_rest;
    float max_velocity;
} player_transform_inputs;

typedef struct bullet_transform_inputs_
{
    float2 forward_vector;
    float speed;
} bullet_transform_inputs;

typedef struct asteroid_transform_inputs_
{
    float2 forward_vector;
    float forward_speed;
    float rotation;
    float rotation_speed;
} asteroid_transform_inputs;

typedef struct actor_transform_outputs_
{
    float2 position;
    float rotation;
    float2 scale;
} actor_transform_outputs;


typedef struct actor_id_
{
    size_t id;
} actor_id;


float float_max (const float a, const float b);
float float_min (const float a, const float b);
float float_clamp (const float value, const float min, const float max);
float float2_length (const float2* in_vector);
void float2_normalize (float2* in_vector);

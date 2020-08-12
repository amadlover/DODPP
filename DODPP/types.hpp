#pragma once


struct float2
{
    float2 ()
    {
        x = 0;
        y = 0;
    }
    float2 (float x_, float y_)
    {
        x = x_;
        y = y_;
    }
    
    float x;
    float y;
};

struct float3
{
    float3 () 
    {
        x = 0;
        y = 0;
        z = 0;
    }
    float3 (float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    float x;
    float y;
    float z;
};

struct player_transform_inputs
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
};

struct bullet_transform_inputs
{
    bullet_transform_inputs ()
    {
        forward_vector = float2 (0, 1);
        speed = 0;
    }

    bullet_transform_inputs (float2 forward_vector_, float speed_)
    {
        forward_vector = forward_vector_;
        speed = speed_;
    }

    float2 forward_vector;
    float speed;
};

struct asteroid_transform_inputs
{
    asteroid_transform_inputs ()
    {
        forward_vector = float2 (0, 1);
        forward_speed = 0;
        rotation = 0;
        rotation_speed = 0;
    }

    asteroid_transform_inputs (float2 forward_vector_, float forward_speed_, float rotation_, float rotation_speed_)
    {
        forward_vector = forward_vector_;
        forward_speed = forward_speed_;
        rotation = rotation_;
        rotation_speed = rotation_speed_;
    }

    float2 forward_vector;
    float forward_speed;
    float rotation;
    float rotation_speed;
};

struct actor_transform_outputs
{
    actor_transform_outputs ()
    {
        position = float2 (0, 0);
        rotation = 0;
        scale = float2 (1, 1);
    }

    actor_transform_outputs (float2 position_, float rotation_, float2 scale_)
    {
        position = position_;
        rotation = rotation_;
        scale = scale_;
    }

    float2 position;
    float rotation;
    float2 scale;
};


float float2_length (const float2* in_vector);

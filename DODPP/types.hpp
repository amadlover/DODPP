#pragma once

struct vec2
{
    vec2 () {}
    vec2 (float x_, float y_)
    {
        x = x_;
        y = y_;
    }

    float x;
    float y;
};

struct vec3
{
    vec3 () {}
    vec3 (float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    float x;
    float y;
    float z;
};

struct position_inputs
{
    position_inputs () {}
    position_inputs (vec2 position_, vec2 direction_, float speed_)
    {
        position = position_;
        direction = direction_;
        speed = speed_;
    }

    vec2 position;
    vec2 direction;
    float speed;
};;

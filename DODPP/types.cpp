#include "types.hpp"

#include <cmath>

float float2_length (const float2* in_vector)
{
    return hypotf (in_vector->x, in_vector->y);
}

void float2_unit_vector (float2* in_out_vector)
{
    float length = hypot (in_out_vector->x, in_out_vector->y);

    in_out_vector->x /= length;
    in_out_vector->y /= length;
}

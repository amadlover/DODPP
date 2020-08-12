#include "types.hpp"

#include <cmath>

float float2_length (const float2* in_vector)
{
    return hypotf (in_vector->x, in_vector->y);
}
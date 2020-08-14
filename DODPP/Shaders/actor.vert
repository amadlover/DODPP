#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform mat_ubo
{
    vec2 actor_position;
    float actor_rotation;
    vec2 actor_scale;
} mat_buff;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec2 out_uv;

void main ()
{
    vec3 scaled_pos = vec3 (mat_buff.actor_scale.x * in_position.x,
                            mat_buff.actor_scale.y * in_position.y,
                            in_position.z);

    vec3 rotated_pos = vec3 ((scaled_pos.x * cos (mat_buff.actor_rotation)) - (scaled_pos.y * sin (mat_buff.actor_rotation)),
                             (scaled_pos.y * cos (mat_buff.actor_rotation)) + (scaled_pos.x * sin (mat_buff.actor_rotation)), 
                             scaled_pos.z
                            );
    
    vec3 final_pos = rotated_pos + vec3 (mat_buff.actor_position, 0);

    gl_Position = vec4 (final_pos, 1);

    out_uv = in_uv;
}

#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D color_texture[4];
layout (push_constant) uniform f_push_constants
{
    layout (offset = 4) uint image_index;
} f_constants;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main ()
{
    out_color = texture (color_texture[f_constants.image_index], in_uv);
}    

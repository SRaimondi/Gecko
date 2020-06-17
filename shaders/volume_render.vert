#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 position_view_space;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    vec4 p = view_matrix * model_matrix * vec4(in_position, 1.f);
    position_view_space = p.xyz;
    gl_Position = projection_matrix * p;
}
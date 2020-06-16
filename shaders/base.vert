#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

out vec3 color;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    color = in_color;
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1.f);
}
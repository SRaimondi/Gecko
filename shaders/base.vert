#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

// out vec3 position;
out vec3 normal;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    normal = normal_matrix * in_normal;
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1.f);
}
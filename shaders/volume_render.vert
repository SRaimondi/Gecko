#version 330 core

layout (location = 0) in vec3 in_position;

out vec4 position_view_space;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    position_view_space = view_matrix * model_matrix * vec4(in_position, 1.f);
    gl_Position = projection_matrix * position_view_space;
}

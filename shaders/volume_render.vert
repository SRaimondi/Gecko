#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 p_model_space;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    p_model_space = in_position;
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1.f);
}

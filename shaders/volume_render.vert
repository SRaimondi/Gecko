#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 p_model_space;

uniform mat4 MVP;

void main() {
    p_model_space = in_position;
    gl_Position = MVP * vec4(in_position, 1.f);
}

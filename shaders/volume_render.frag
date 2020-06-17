#version 330 core

out vec4 fragment_color;

in vec3 position_view_space;

void main() {
    fragment_color = vec4(abs(normalize(position_view_space)), 1.f);
}
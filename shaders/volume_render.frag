#version 330 core

out vec4 fragment_color;

in vec4 position_view_space;

void main() {
    fragment_color = vec4(abs(normalize(position_view_space.xyz)), 1.f);
}
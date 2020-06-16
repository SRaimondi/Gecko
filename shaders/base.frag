#version 330 core

out vec4 fragment_color;

in vec3 color;

void main() {
    fragment_color = vec4(color, 1.f);
}
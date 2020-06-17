#version 330 core

out vec4 fragment_color;

in vec3 normal;

void main() {
    fragment_color = vec4(abs(normal), 1.f);
}
#version 330 core

out vec4 fragment_color;

in vec3 p_model_space;

uniform vec3 eye_model_space;
uniform float step_size;

uniform sampler3D volume_texture;

vec2 compute_bounds_hit(in vec3 ray_origin, in vec3 inv_ray_direction,
in vec3 bounds_min, in vec3 bounds_max) {
    vec3 bounds_min_intersection = (bounds_min - ray_origin) * inv_ray_direction;
    vec3 bounds_max_intersection = (bounds_max - ray_origin) * inv_ray_direction;
    vec3 slabs_min_intersection = min(bounds_min_intersection, bounds_max_intersection);
    vec3 slabs_max_intersection = max(bounds_min_intersection, bounds_max_intersection);
    return vec2(
    max(slabs_min_intersection.x, max(slabs_min_intersection.y, slabs_min_intersection.z)),
    min(slabs_max_intersection.x, min(slabs_max_intersection.y, slabs_max_intersection.z))
    );
}

void main() {
    vec3 dir = normalize(p_model_space - eye_model_space);
    vec2 t = compute_bounds_hit(eye_model_space, vec3(1.f) / dir, vec3(0.f), vec3(1.f));

    vec3 attenuation = vec3(0.f);
    float current_t = t.x;
    vec3 current_point = eye_model_space + current_t * dir;
    vec3 step = step_size * dir;

    while (current_t <= t.y) {
        vec4 volume_value = texture(volume_texture, current_point);
        attenuation += volume_value.x * step_size;
        current_t += step_size;
        current_point += step;
    }

    fragment_color = vec4(exp(-attenuation), 1.f);
}
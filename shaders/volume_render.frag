#version 330 core

out vec4 fragment_color;

in vec3 p_model_space;

uniform vec3 eye_model_space;
uniform float step_size;

uniform sampler3D volume_texture;
uniform sampler1D transfer_function_texture;

float minElement(in vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float maxElement(in vec3 v) {
    return max(v.x, max(v.y, v.z));
}

vec2 computeBoundsHit(in vec3 ray_origin, in vec3 inv_ray_direction,
in vec3 bounds_min, in vec3 bounds_max) {
    vec3 bounds_min_intersection = (bounds_min - ray_origin) * inv_ray_direction;
    vec3 bounds_max_intersection = (bounds_max - ray_origin) * inv_ray_direction;
    vec3 slabs_min_intersection = min(bounds_min_intersection, bounds_max_intersection);
    vec3 slabs_max_intersection = max(bounds_min_intersection, bounds_max_intersection);

    return vec2(maxElement(slabs_min_intersection), minElement(slabs_max_intersection));
}

void main() {
    vec3 dir = normalize(p_model_space - eye_model_space);
    vec2 t = computeBoundsHit(eye_model_space, vec3(1.f) / dir, vec3(0.f), vec3(1.f));

    vec3 attenuation = vec3(0.f);
    float current_t = t.x;
    vec3 current_point = eye_model_space + current_t * dir;
    vec3 step = step_size * dir;

    while (current_t <= t.y && any(lessThan(attenuation, vec3(10.f)))) {
        vec4 volume_value = texture(volume_texture, current_point);
        attenuation += volume_value.r * step_size;
        current_t += step_size;
        current_point += step;
    }

    fragment_color = vec4(exp(-attenuation), 1.f);
}
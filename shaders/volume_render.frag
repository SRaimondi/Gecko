#version 330 core

out vec4 fragment_color;

in vec4 p_model_space;

uniform vec3 eye_model_space;

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
    vec3 dir = normalize(p_model_space.xyz - eye_model_space);
    vec2 t = compute_bounds_hit(eye_model_space, vec3(1.f) / dir, vec3(0.f), vec3(1.f));

    if (t.x <= t.y) {
        fragment_color = vec4(1.f, 1.f, 0.f, 1.f);
    } else {
        fragment_color = vec4(0.f, 0.f, 1.f, 1.f);
    }
}
#version 330 core

out vec4 fragment_color;

in vec3 p_model_space;

uniform vec3 eye_model_space;
uniform float step_size;

uniform sampler3D volume_texture;
uniform sampler3D volume_normal_texture;

uniform float min_value;
uniform float mult;

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
/*
vec3 computeGradient(in vec3 position) {
    float grad_eps = step_size;
    float inv_2_grad_eps = 1.f / (2.f * grad_eps);
    float dx = (texture(volume_texture, position + vec3(grad_eps, 0.f, 0.f)).r -
                texture(volume_texture, position - vec3(grad_eps, 0.f, 0.f)).r) *
                inv_2_grad_eps;
    float dy = (texture(volume_texture, position + vec3(0.f, grad_eps, 0.f)).r -
                texture(volume_texture, position - vec3(0.f, grad_eps, 0.f)).r) *
                inv_2_grad_eps;
    float dz = (texture(volume_texture, position + vec3(0.f, 0.f, grad_eps)).r -
                texture(volume_texture, position - vec3(0.f, 0.f, grad_eps)).r) *
                inv_2_grad_eps;

    return vec3(dx, dy, dz);
}*/

void main() {
    vec3 dir = normalize(p_model_space - eye_model_space);
    vec2 t = computeBoundsHit(eye_model_space, vec3(1.f) / dir, vec3(0.f), vec3(1.f));

    float alpha = 0.f;
    vec3 c = vec3(0.f);

    float current_t = t.x;
    vec3 current_point = eye_model_space + current_t * dir;
    vec3 step = step_size * dir;

    while (current_t <= t.y && alpha < 0.99f) {
        float score_value = texture(volume_texture, current_point).r;
        vec3 normal = normalize(texture(volume_normal_texture, current_point).xyz);

        // Map volume value to tf interval
        vec4 tf_value = vec4(0.f);

        if (score_value >= min_value) {
            tf_value.rgb = mult * vec3(abs(dot(-dir, normal))) * abs(normal);
            tf_value.a = 1.f;
        } else {
            tf_value.rgb = 0.1f * mult * vec3(score_value);
            tf_value.a = score_value;
        }
        // Correct based on step size
        float alpha_p = 1.f - pow(1.f - tf_value.a, step_size);
        vec3 color_p = tf_value.rgb * step_size;

        // Accumulate
        c = c + (1.f - alpha) * color_p;
        alpha = alpha + (1.f - alpha) * alpha_p;

        current_t += step_size;
        current_point += step;
    }

    fragment_color = vec4(c, 1.f);
}
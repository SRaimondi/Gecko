#version 330 core

out vec4 fragment_color;

in vec4 p_projection_space;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 inverse_view_projection_matrix;

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
    mat4 inverse_model_matrix = inverse(model_matrix);


    vec3 eye = -transpose(mat3(view_matrix)) * view_matrix[3].xyz;
    vec3 m_eye = (inverse_model_matrix * vec4(eye, 1.f)).xyz;
    vec4 dir = inverse_view_projection_matrix * vec4(p_projection_space.xyz, 1.f);
    vec3 m_dir = (inverse_model_matrix * vec4(dir.xyz, 0.f)).xyz;
//    vec3 b_min = (model_matrix * vec4(-1.f, -1.f, -1.f, 1.f)).xyz;
//    vec3 b_max = (model_matrix * vec4(1.f)).xyz;

    vec2 t = compute_bounds_hit(eye, vec3(1.f) / dir.xyz, vec3(-1.f), vec3(1.f));

    if (t.x <= t.y) {
        fragment_color = vec4(1.f, 1.f, 0.f, 1.f);
    } else {
        fragment_color = vec4(0.f, 0.f, 1.f, 1.f);
    }

//    fragment_color = vec4(abs(normalize(dir.xyz)), 1.f);
//    fragment_color = vec4(clamp(eye, 0.f, 1.f), 1.f);
}
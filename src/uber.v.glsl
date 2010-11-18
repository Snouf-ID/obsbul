#version 330

#ifdef POSITION
// Attribute indexes automatically defined in EntityFactory.
layout (location = POSITION) in vec4 in_position;
layout (location = NORMAL) in vec4 in_normal;
layout (location = TEXCOORD) in vec2 in_texcoord;
layout (location = MATERIAL_IDX) in unsigned int in_material_idx;

uniform mat4 model;
uniform mat4 normalmatrix;
uniform mat4 mvp;

#endif // POSITION

mat4 view_frustum(
    float angle_of_view,
    float aspect_ratio,
    float z_near,
    float z_far
) {
    return mat4(
        vec4(1.0/tan(angle_of_view),           0.0, 0.0, 0.0),
        vec4(0.0, aspect_ratio/tan(angle_of_view),  0.0, 0.0),
        vec4(0.0, 0.0,    (z_far+z_near)/(z_far-z_near), 1.0),
        vec4(0.0, 0.0, -2.0*z_far*z_near/(z_far-z_near), 0.0)
    );
}

mat4 scale(float factor)
{
    return mat4(
        vec4(factor, 0.0, 0.0, 0.0),
        vec4(0.0, factor, 0.0, 0.0),
        vec4(0.0, 0.0, factor, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

mat4 rotate_x(float theta)
{
    return mat4(
        vec4(1.0,         0.0,         0.0, 0.0),
        vec4(0.0,  cos(theta),  sin(theta), 0.0),
        vec4(0.0, -sin(theta),  cos(theta), 0.0),
        vec4(0.0,         0.0,         0.0, 1.0)
    );
}

mat4 rotate_y(float theta)
{
    return mat4(
        vec4(cos(theta), 0.0, -sin(theta), 0.0),
        vec4(0.0,        1.0,  sin(theta), 0.0),
        vec4(sin(theta), 0.0,  cos(theta), 0.0),
        vec4(0.0,        0.0,         0.0, 1.0)
    );
}

mat4 translate(float x, float y, float z)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(x,   y,   z,   1.0)
    );
}

out vec4 frag_diffuse;
out vec4 frag_specular;
out float frag_shininess;
out vec3 frag_worldspace_pos;
out vec3 frag_normal;
out vec2 frag_texcoord;

#ifdef MATERIALS

struct Material {
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

layout(std140) uniform materials {
    Material Materials[MATERIALS];
};

#endif // MATERIALS

void main(void)
{
    #ifdef FRUSTUM
    gl_Position = mvp
                #ifdef HALFSIZE
                * scale(0.5)
                #elif defined TINYSIZE
                * scale(0.1)
                #endif // SIZE
                * in_position;
    #endif // FRUSTUM
    
    #ifdef ORTHO
    gl_Position = in_position;
    #endif ORTHO
    frag_normal = (normalmatrix * in_normal).xyz;
    frag_texcoord = in_texcoord;
    frag_worldspace_pos = (model * in_position).xyz;
    #ifdef MATERIALS
    frag_diffuse = Materials[in_material_idx].diffuse;
    frag_specular = Materials[in_material_idx].specular;
    frag_shininess = Materials[in_material_idx].shininess;
    #endif // MATERIALS
}

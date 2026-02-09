#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;

uniform mat4 u_projection;
uniform mat4 u_model;


uniform vec3 u_lightPosition; // The position of your light source

uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;
uniform mat4 u_orthoProjection;
uniform bool u_toggleOrtho;


// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec4 a_normal;
// ...


// Vertex shader outputs
// ...
out vec4 v_color;
out vec3 v_normal;


void main()
{
    mat4 MVP = mat4(1.0);
    if (u_toggleOrtho){
        MVP = u_orthoProjection * u_view * u_model;
    }else{
        MVP = u_projection * u_view * u_model;
    }
    
    mat4 mv = u_view * u_model;
    gl_Position = MVP * a_position;
    v_normal = normalize(mat3(mv) * a_normal.xyz);
    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

    // Calculate the view-space normal
    vec3 N = normalize(mat3(mv) * a_normal.xyz);

    // Calculate the view-space light direction
    vec3 L = normalize(u_lightPosition - positionEye);

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));

    // Multiply the diffuse reflection term with the base surface color
    //v_color = vec4(diffuse * u_diffuseColor, 1.0);
    vec3 V = normalize(-positionEye);
    vec3 H = (L + V)/length(L+V);
    v_color = vec4(
        u_ambientColor
        + u_diffuseColor * L * diffuse
        + u_specularColor * L * pow(max(dot(N, H), 0.0), u_specularPower),
        1.0
    );
}


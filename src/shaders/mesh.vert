#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;

uniform mat4 u_projection;
uniform mat4 u_model;

uniform vec3 u_diffuseColor; // The diffuse surface color of the model
uniform vec3 u_lightPosition; // The position of your light source


// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec4 a_normal;
// ...


// Vertex shader outputs
// ...
out vec4 v_color;


void main()
{
    mat4 MVP = u_projection * u_view * u_model;
    mat4 mv = u_view * u_model;
    gl_Position = MVP * a_position;
    v_color = 0.5 * a_normal + 0.5;
       
    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

    // Calculate the view-space normal
    vec3 N = normalize(mv * a_normal).xyz;

    // Calculate the view-space light direction
    vec3 L = normalize(u_lightPosition - positionEye);

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));

    // Multiply the diffuse reflection term with the base surface color
    v_color = vec4(u_diffuseColor*diffuse, 1.0);
        
    
}

#version 330
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_diffuseColor;
uniform vec3 u_lightPosition;

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec3 a_normal; // vec3, not vec4

out vec4 v_color;

void main()
{
    // Model-view-projection
    mat4 mv = u_view * u_model;
    mat4 MVP = u_projection * mv;
    gl_Position = MVP * a_position;

    // Transform position to view space
    vec3 positionEye = vec3(mv * a_position);

    // Transform normal correctly
    mat3 normalMatrix = transpose(inverse(mat3(mv)));
    vec3 N = normalize(normalMatrix * a_normal);

    // Light direction in view space
    vec3 lightEye = vec3(u_view * vec4(u_lightPosition, 1.0));
    vec3 L = normalize(lightEye - positionEye);

    // Lambert diffuse
    float diffuse = max(dot(N, L), 0.0);

    // Final color
    v_color = vec4(u_diffuseColor * diffuse, 1.0);
}

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
uniform bool u_toggleReflective;


// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec4 a_normal;
layout(location = 3) in vec2 a_texcoord_0;
// ...


// Vertex shader outputs
// ...
out vec4 v_color;
out vec3 v_normal;
out vec4 v_position;
out vec3 v_positionEye;
out vec2 v_texcoord_0;

//look at what grok yapped abt last time
//gl gng o7

void main()
{
    mat4 MVP = mat4(1.0);
    if (u_toggleOrtho){
        MVP = u_orthoProjection * u_view * u_model;
    }else{
        MVP = u_projection * u_view * u_model;
    }
    
    mat4 mv = u_view * u_model;
        // Transform the vertex position to view space (eye coordinates)
    v_positionEye = vec3(mv * a_position);

    // Calculate the view-space light direction
    
    gl_Position = MVP * a_position;
    v_normal = normalize(mat3(mv) * a_normal.xyz);
    v_texcoord_0 = a_texcoord_0;
    //gradient for test
    //v_texcoord_0 = a_position.xy * 0.5 + 0.5;
    v_position = mv * a_position;
}


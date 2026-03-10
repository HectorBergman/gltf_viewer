#version 330 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec4 a_normal;  

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_outlineThickness;

void main() {
    vec3 expanded = a_position.xyz + a_normal.xyz * u_outlineThickness;
    gl_Position = u_projection * u_view * u_model * vec4(expanded, 1.0);
}
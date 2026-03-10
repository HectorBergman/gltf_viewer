#version 330 core
uniform vec3 u_wireframeColor;
out vec4 fragColor;

void main() {
    fragColor = vec4(u_wireframeColor, 1.0);
}
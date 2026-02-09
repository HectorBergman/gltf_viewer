#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
// ...
uniform bool u_showNormals;
uniform bool u_toggleOrtho;
// Fragment shader inputs
// ...
in vec4 v_color;
in vec3 v_normal;
// Fragment shader outputs
out vec4 frag_color;

void main()
{
    if (u_showNormals) {
        vec3 N = normalize(v_normal);
        frag_color = vec4(N * 0.5 + 0.5, 1.0);
        return;
    }
    frag_color = v_color;
}

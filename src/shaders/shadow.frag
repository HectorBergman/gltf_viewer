#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
// ...

// Fragment shader inputs
// ...

// Fragment shader outputs
out vec4 frag_color;

void main()
{
    float depth = gl_FragCoord.z;
    float remapped = 1.0 - (1.0 - depth) * 25.0; // tweak the multiplier
    frag_color = vec4(remapped, remapped, remapped, 1.0);
}

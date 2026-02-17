#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
// ...
uniform bool u_showNormals;
uniform bool u_toggleOrtho;

uniform vec3 u_lightPosition;
uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;
// Fragment shader inputs
// ...
in vec4 v_color;
in vec3 v_normal;
in vec4 v_position;
in mat4 v_mv;
in vec3 N;
in vec3 L;
in vec3 V;
// Fragment shader outputs
out vec4 f_color;

void main()
{
    if (u_showNormals) {
        vec3 No = normalize(v_normal);
        f_color = vec4(No * 0.5 + 0.5, 1.0);
        return;
    }



    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    //vec4 f_color = vec4(diffuse * u_diffuseColor, 1.0);
    // Multiply the diffuse reflection term with the base surface color
    
    
    vec3 H = (L + V)/length(L+V);
    vec3 I_s =  (u_specularPower+8)/8 *  u_specularColor * 
            L * (pow(dot(N,H), u_specularPower));
    vec4 f_color = vec4(
        u_ambientColor
        + u_diffuseColor * L * diffuse
        + I_s,
        1.0
    );
    f_color = vec4(pow(f_color.xyz, vec3(1 / 2.2)), 1);
}

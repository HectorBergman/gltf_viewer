#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
// ...
uniform bool u_showNormals;
uniform bool u_toggleOrtho;
uniform bool u_toggleReflective;

uniform vec3 u_lightPosition;
uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;
uniform samplerCube u_cubemap;
// Fragment shader inputs
// ...
in vec4 v_color;
in vec3 v_normal;
in vec4 v_position;
in vec3 v_positionEye;

// Fragment shader outputs
out vec4 f_color;

void main()
{
    if (u_showNormals) {
        vec3 No = normalize(v_normal);
        f_color = vec4(No * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightPosition - v_positionEye);
    vec3 V = normalize(-v_positionEye);
    vec3 H = (L + V)/length(L+V);
    vec3 R = reflect(-V, N);

    vec3 color = texture(u_cubemap, R).rgb;

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    
    // Multiply the diffuse reflection term with the base surface color
    
    
    //todo: go to assets/cubemaps/romechurch, observe that
    //they are 0.125, 0.5, 2, 8, etc., add a slider or some equivalent
    //to go from 0.125, 0.5, 2, 8, etc. to change cubemaps.
    
    vec3 I_s =  (u_specularPower+8)/8 *  u_specularColor * 
            L * (pow(dot(N,H), u_specularPower));
    if (u_toggleReflective) {
        f_color = vec4(color, 1);
    }else{
        f_color = vec4(
            u_ambientColor
            + u_diffuseColor * L * diffuse
            + I_s,
            1.0
        );
    }
    f_color = vec4(color, 1.0);
    //f_color = vec4(pow(f_color.xyz, vec3(1 / 2.2)), 1);
    
}

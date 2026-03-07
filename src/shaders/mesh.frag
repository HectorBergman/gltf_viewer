#version 330
#extension GL_ARB_explicit_attrib_location : require

uniform bool u_showNormals;
uniform bool u_toggleOrtho;
uniform bool u_toggleReflective;
uniform bool u_toggleUV;
uniform bool u_toggleTexture;
uniform bool u_toonShading;
uniform bool u_evil_toonShading;
uniform int u_toon_colorLevels;

uniform vec3 u_lightPosition;
uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;
uniform samplerCube u_cubemap;
uniform sampler2D u_baseColorTexture;

// Shadow mapping uniforms
uniform sampler2D u_shadowmap;
uniform mat4 u_shadowFromView;
uniform float u_shadowBias;

in vec4 v_color;
in vec3 v_normal;
in vec4 v_position;
in vec3 v_positionEye;
in vec2 v_texcoord_0;

out vec4 f_color;

float shadowmap_visibility(sampler2D shadowmap, vec4 shadowPos, float bias)
{
    vec2 delta = vec2(0.5) / textureSize(shadowmap, 0).xy;
    vec2 texcoord = (shadowPos.xy / shadowPos.w) * 0.5 + 0.5;
    float depth = (shadowPos.z / shadowPos.w) * 0.5 + 0.5;

    float visibility = 0.0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(float(x), float(y)) * delta;
            float texel = texture(shadowmap, texcoord + offset).r;
            visibility += float(texel > depth - bias);
        }
    }
    return visibility / 9.0;  // average over 3x3 = 9 samples
}

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
    vec3 diffuseColor = u_diffuseColor;
    if (u_toggleTexture) {
        diffuseColor = texture(u_baseColorTexture, v_texcoord_0).rgb;
    }

    float diffuse = max(0.0, dot(N, L));
    if (u_toonShading) {
        diffuse = ceil(diffuse * float(u_toon_colorLevels)) / float(u_toon_colorLevels);
    }

    vec3 totalDiffuse = diffuse * diffuseColor;

    vec3 I_s = (u_specularPower+8)/8 * u_specularColor *
               (pow(dot(N,H), u_specularPower));

    // Compute shadow visibility
    vec4 shadowPos = u_shadowFromView * vec4(v_positionEye, 1.0);
    float visibility = shadowmap_visibility(u_shadowmap, shadowPos, u_shadowBias);

    vec3 finalColor = u_ambientColor + visibility * (totalDiffuse + I_s);

    if (u_evil_toonShading) {
        finalColor = round(finalColor * float(u_toon_colorLevels)) / float(u_toon_colorLevels);
    }

    if (u_toggleReflective) {
        f_color = vec4(color, 1);
    } else {
        f_color = vec4(finalColor, 1.0);
    }

    if (u_toggleUV) {
        f_color = vec4(v_texcoord_0, .0, 1.0);
    } else {
        f_color.xyz = pow(max(f_color.xyz, 0.0), vec3(1.0 / 2.2));
    }
}
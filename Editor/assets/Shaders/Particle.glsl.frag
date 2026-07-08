#version 460 core

layout(location = 10) uniform sampler2D u_Texture;
layout(location = 11) uniform vec4      u_Color;      // emitter tint (may exceed 1 for bloom)
layout(location = 12) uniform int       u_HasTexture;

layout(location = 0) in vec2 v_TexCoords;

layout(location = 0) out vec4 FragColor;

void main()
{
    // Soft round falloff so each billboard reads as a glowing ember, not a quad.
    float d = length(v_TexCoords - vec2(0.5)) * 2.0;
    float alpha = clamp(1.0 - d, 0.0, 1.0);
    alpha *= alpha;

    vec3 tint = u_Color.rgb;
    if(u_HasTexture == 1)
        tint *= texture(u_Texture, v_TexCoords).rgb;

    // Additive blend into the HDR buffer (feeds the bloom pass).
    FragColor = vec4(tint * alpha, alpha);
}

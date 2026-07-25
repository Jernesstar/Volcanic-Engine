#version 460 core

layout(location = 10) uniform sampler2D u_Texture;
layout(location = 11) uniform vec4      u_Color;      // emitter tint (may exceed 1 for bloom)
layout(location = 12) uniform int       u_HasTexture;

// Authored 3-stop age colour ramp (young -> mid -> old). HDR-valued: young
// particles can write above the bloom threshold and glow; old ones fall below it
// and fade — the emitter's brightest end blooms for free from the bloom chain.
// Neutral white defaults leave u_Color unchanged. Additive blending cannot render
// dark fragments, so particles fade out rather than turning grey.
// Locations continue past the vertex stage's u_ParticleLifetime (location 13):
// uniform locations share a namespace across linked stages, so 14/15/16 avoid it.
layout(location = 14) uniform vec3      u_ColorStart; // at spawn (age 0)
layout(location = 15) uniform vec3      u_ColorMid;   // mid-life
layout(location = 16) uniform vec3      u_ColorEnd;   // at death (age 1)

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) in float v_Age;

layout(location = 0) out vec4 FragColor;

vec3 AgeRamp(float t) {
    return t < 0.4
        ? mix(u_ColorStart, u_ColorMid, t / 0.4)
        : mix(u_ColorMid, u_ColorEnd, (t - 0.4) / 0.6);
}

void main()
{
    float d = length(v_TexCoords - vec2(0.5)) * 2.0;

    // Alpha quantized to bands: a smooth radial gradient across a few pixels
    // turns to mush at 320x180; crisp bands read as pixel-art sprites.
    const float k_Bands = 4.0;
    float alpha = clamp(1.0 - d, 0.0, 1.0);
    alpha = floor(alpha * k_Bands) / k_Bands;

    // Ramp first, emitter tint multiplies it, then an optional texture modulates
    // (procedural is the default and only validated path this sprint).
    vec3 tint = AgeRamp(v_Age) * u_Color.rgb;
    if(u_HasTexture == 1)
        tint *= texture(u_Texture, v_TexCoords).rgb;

    // Additive blend into the HDR buffer (feeds the bloom pass).
    FragColor = vec4(tint * alpha, alpha);
}

#version 460 core
// Nearest-neighbour upscale from the low-res pixel-art buffer to full-res,
// with a sub-pixel UV nudge for smooth isometric panning.
// Bloom is composited here and tone-mapped in the same pass.

layout(binding = 0) uniform sampler2D u_LowResScene; // 320x180, nearest
layout(binding = 1) uniform sampler2D u_Bloom;        // same low-res, nearest

layout(location = 0) uniform vec2  u_SubPixelOffset;  // in [0,1) UV space of low-res
layout(location = 1) uniform float u_SrcWidth;        // 320.0
layout(location = 2) uniform float u_SrcHeight;       // 180.0
layout(location = 3) uniform float u_Exposure;        // default 1.0
layout(location = 4) uniform float u_BloomStrength;   // default 0.04

layout(location = 0) in vec2 v_TexCoords;
layout(location = 0) out vec4 FragColor;

// The sub-pixel shift is: snap(uv * lowResSize) / lowResSize gives the
// nearest texel; adding back the fractional remainder sub-texel nudge
// shifts the sample point smoothly within the nearest-neighbour grid.
void main()
{
	// Map full-res UV → low-res texel centre (nearest), then apply offset
	vec2 lowResSize = vec2(u_SrcWidth, u_SrcHeight);

	// Pixel-snapped UV: land exactly on a texel boundary
	vec2 snappedUV = (floor(v_TexCoords * lowResSize) + 0.5) / lowResSize;

	// Sub-pixel nudge: shift by < 1 texel for smooth camera pan
	vec2 uv = snappedUV + u_SubPixelOffset;

	vec3 scene = texture(u_LowResScene, uv).rgb;
	vec3 bloom = texture(u_Bloom, uv).rgb;

	vec3 result = mix(scene, bloom, u_BloomStrength);

	// Reinhard tone map + gamma
	result = vec3(1.0) - exp(-result * u_Exposure);
	result = pow(result, vec3(1.0 / 2.2));

	FragColor = vec4(result, 1.0);
}
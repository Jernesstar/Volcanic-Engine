#version 460 core

layout(binding = 0) uniform sampler2D u_HDR;
layout(binding = 1) uniform sampler2D u_Bloom;

layout(location = 0) uniform float u_Exposure      = 1.0;
layout(location = 1) uniform float u_BloomStrength = 0.04;
// Sub-pixel UV shift injected by the isometric pixel-snap hook.
// Zero when the hook is not active (standard perspective pipeline).
layout(location = 2) uniform vec2  u_SubPixelOffset = vec2(0.0);

layout(location = 0) in  vec2 v_TexCoords;
layout(location = 0) out vec4 FragColor;

// Narkowicz ACES filmic approximation
// Preserves hue better than plain Reinhard under high exposure
vec3 ACES(vec3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	vec2 uv = v_TexCoords + u_SubPixelOffset;

	vec3 hdr   = texture(u_HDR,   uv).rgb;
	vec3 bloom = texture(u_Bloom, uv).rgb;

	// Additive bloom composite
	vec3 color = hdr + bloom * u_BloomStrength;

	// Exposure
	color *= u_Exposure;

	// ACES filmic tone map
	color = ACES(color);

	// Gamma correction (linear → sRGB)
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}
#version 460 core

layout(binding = 0) uniform sampler2D u_HDR;
layout(binding = 1) uniform sampler2D u_Bloom;

layout(location = 0) uniform float u_Exposure      = 1.0;
layout(location = 1) uniform float u_BloomStrength = 0.04;
layout(location = 2) uniform vec2  u_SubPixelOffset = vec2(0.0);
layout(location = 3) uniform float u_SrcWidth      = 1920.0;
layout(location = 4) uniform float u_SrcHeight     = 1080.0;

layout(location = 0) in  vec2 v_TexCoords;
layout(location = 0) out vec4 FragColor;

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
	vec2 lowResSize = vec2(u_SrcWidth, u_SrcHeight);
	vec2 snappedUV = (floor(v_TexCoords * lowResSize) + 0.5) / lowResSize;
	vec2 uv = snappedUV + u_SubPixelOffset;

	ivec2 hdrCoord = clamp(ivec2(uv * lowResSize), ivec2(0),
		ivec2(lowResSize) - ivec2(1));
	vec2 bloomSize = vec2(textureSize(u_Bloom, 0));
	ivec2 bloomCoord = clamp(ivec2(uv * bloomSize), ivec2(0),
		ivec2(bloomSize) - ivec2(1));

	vec3 hdr   = texelFetch(u_HDR,   hdrCoord, 0).rgb;
	vec3 bloom = texelFetch(u_Bloom, bloomCoord, 0).rgb;

	vec3 color = hdr + bloom * u_BloomStrength;
	color *= u_Exposure;
	color = ACES(color);
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
	// FragColor = vec4(v_TexCoords, 1.0, 1.0);
}

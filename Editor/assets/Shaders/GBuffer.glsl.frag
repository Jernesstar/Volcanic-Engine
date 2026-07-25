#version 460 core

// Material uniforms (set by name from the geometry pass).
layout(location = 4) uniform sampler2D u_Albedo;
layout(location = 5) uniform vec4      u_AlbedoColor;
layout(location = 6) uniform int       u_HasAlbedoTexture;
layout(location = 7) uniform float     u_Emissive; // 0..1, emissive surfaces > 0

layout(location = 0) in vec3 v_FragPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoords;

// Outputs: three G-Buffer targets
layout(location = 0) out vec4 g_Position;
layout(location = 1) out vec4 g_Normal;
layout(location = 2) out vec4 g_Albedo; // rgb = albedo, a = emissive strength

void main()
{
	g_Position = vec4(v_FragPos, 1.0);
	g_Normal = vec4(normalize(v_Normal), 1.0);

	vec3 albedo;
	if(u_HasAlbedoTexture == 1)
		albedo = texture(u_Albedo, v_TexCoords).rgb;
	else
		albedo = u_AlbedoColor.rgb;

	g_Albedo = vec4(albedo, u_Emissive);
}
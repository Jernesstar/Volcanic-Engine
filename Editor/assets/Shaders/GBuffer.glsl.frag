#version 460 core

layout(location = 0) in vec3 v_FragPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoords;

// Material uniforms (bound by MaterialBinder)
layout(location = 4) uniform sampler2D u_Albedo;
layout(location = 5) uniform vec4      u_AlbedoColor;
layout(location = 6) uniform int       u_HasAlbedoTexture;
layout(location = 7) uniform float     u_Emissive; // 0..1, torches > 0

// Outputs: three G-Buffer targets
layout(location = 0) out vec3 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec4 g_Albedo; // rgb = albedo, a = emissive strength

void main()
{
	g_Position = v_FragPos;
	g_Normal   = normalize(v_Normal);

	vec3 albedo;
	if(u_HasAlbedoTexture == 1)
		albedo = texture(u_Albedo, v_TexCoords).rgb;
	else
		albedo = u_AlbedoColor.rgb;

	g_Albedo = vec4(albedo, u_Emissive);
}
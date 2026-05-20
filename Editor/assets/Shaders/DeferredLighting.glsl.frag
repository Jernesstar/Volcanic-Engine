#version 460 core

// ── G-Buffer inputs ───────────────────────────────────────────────────────────
layout(binding = 0) uniform sampler2D u_GPosition;
layout(binding = 1) uniform sampler2D u_GNormal;
layout(binding = 2) uniform sampler2D u_GAlbedo;   // a = emissive
layout(binding = 3) uniform sampler2D u_ShadowMap;

layout(location = 0) uniform mat4 u_LightSpaceMatrix;
layout(location = 1) uniform vec3 u_CameraPos;

// ── Lights ────────────────────────────────────────────────────────────────────
struct DirLight {
	vec3 Direction;
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};
struct PointLight {
	vec3  Position;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
};

layout(location = 4)  uniform int      u_DirLightCount;
layout(location = 5)  uniform DirLight u_DirLights[4];
layout(location = 21) uniform int      u_PointLightCount;
layout(location = 22) uniform PointLight u_PointLights[16];

// ── In / Out ──────────────────────────────────────────────────────────────────
layout(location = 0) in vec2 v_TexCoords;
layout(location = 0) out vec4 FragColor;

// ── Shadow helpers ────────────────────────────────────────────────────────────
float ShadowFactor(vec3 fragPos, vec3 normal, vec3 lightDir)
{
	vec4 lightSpacePos = u_LightSpaceMatrix * vec4(fragPos, 1.0);
	vec3 proj = lightSpacePos.xyz / lightSpacePos.w;
	proj = proj * 0.5 + 0.5;
	if(proj.z > 1.0)
		return 0.0;

	float bias    = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
	float shadow  = 0.0;
	vec2  texelSz = 1.0 / textureSize(u_ShadowMap, 0);

	// 3×3 PCF
	for(int x = -1; x <= 1; x++)
	for(int y = -1; y <= 1; y++) {
		float pcfDepth = texture(u_ShadowMap,
			proj.xy + vec2(x, y) * texelSz).r;
		shadow += proj.z - bias > pcfDepth ? 1.0 : 0.0;
	}
	return shadow / 9.0;
}

// ── Directional light ─────────────────────────────────────────────────────────
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,
                  vec3 albedo, vec3 fragPos)
{
	vec3 lightDir = normalize(-light.Direction);
	float diff    = max(dot(normal, lightDir), 0.0);
	vec3 reflDir  = reflect(-lightDir, normal);
	float spec    = pow(max(dot(viewDir, reflDir), 0.0), 32.0);

	vec3 ambient  = light.Ambient  * albedo;
	vec3 diffuse  = light.Diffuse  * diff * albedo;
	vec3 specular = light.Specular * spec;

	float shadow = ShadowFactor(fragPos, normal, lightDir);
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

// ── Point light ───────────────────────────────────────────────────────────────
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos,
                    vec3 viewDir, vec3 albedo)
{
	vec3  lightDir = normalize(light.Position - fragPos);
	float diff     = max(dot(normal, lightDir), 0.0);
	vec3  reflDir  = reflect(-lightDir, normal);
	float spec     = pow(max(dot(viewDir, reflDir), 0.0), 32.0);

	float dist  = length(light.Position - fragPos);
	float atten = 1.0 / (light.Constant
	            + light.Linear    * dist
	            + light.Quadratic * dist * dist);

	vec3 ambient  = light.Ambient  * albedo * atten;
	vec3 diffuse  = light.Diffuse  * diff   * albedo * atten;
	vec3 specular = light.Specular * spec           * atten;
	return ambient + diffuse + specular;
}

// ── Main ──────────────────────────────────────────────────────────────────────
void main()
{
	vec3  fragPos  = texture(u_GPosition, v_TexCoords).rgb;
	vec3  normal   = normalize(texture(u_GNormal,  v_TexCoords).rgb);
	vec4  albedoA  = texture(u_GAlbedo,   v_TexCoords);
	vec3  albedo   = albedoA.rgb;
	float emissive = albedoA.a;

	vec3  viewDir  = normalize(u_CameraPos - fragPos);
	vec3  result   = vec3(0.0);

	for(int i = 0; i < u_DirLightCount; i++)
		result += CalcDirLight(u_DirLights[i], normal, viewDir, albedo, fragPos);

	for(int i = 0; i < u_PointLightCount; i++)
		result += CalcPointLight(u_PointLights[i], normal, fragPos, viewDir, albedo);

	// Emissive contribution (feeds bloom extraction in the blur pass)
	result += albedo * emissive * 2.0;

	FragColor = vec4(result, 1.0);
}
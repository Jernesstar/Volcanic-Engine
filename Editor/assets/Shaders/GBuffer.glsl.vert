
#version 460 core

layout(location = 0) uniform mat4 u_ViewProj;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in mat4 a_Transform;  // instanced

layout(location = 0) out vec3 v_FragPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoords;

void main()
{
	vec4 world = a_Transform * vec4(a_Position, 1.0);
	v_FragPos  = world.xyz;
	v_Normal   = mat3(transpose(inverse(a_Transform))) * a_Normal;
	v_TexCoords = a_TexCoords;
	gl_Position = u_ViewProj * world;
}
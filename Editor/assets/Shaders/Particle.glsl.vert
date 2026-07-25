#version 460 core

struct Particle {
    vec3 Position;
    vec3 Velocity;
    float Life;
};

layout(std430, binding = 0) readonly restrict buffer SSBO_0 {
    Particle Particles[];
};

layout(location = 0) uniform mat4 u_View;
layout(location = 4) uniform mat4 u_ViewProj;
layout(location = 8) uniform float u_BillboardWidth;
layout(location = 9) uniform float u_BillboardHeight;
// Location 13: 10-12 are taken by the fragment stage (u_Texture/u_Color/u_HasTexture).
layout(location = 13) uniform float u_ParticleLifetime;

const vec2 Vertices[4] =
    vec2[4](
        vec2(-0.5f, -0.5f),
        vec2( 0.5f, -0.5f),
        vec2( 0.5f,  0.5f),
        vec2(-0.5f,  0.5f)
    );

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) out float v_Age; // normalized age, consumed by the frag stage

void main()
{
    Particle particle = Particles[gl_InstanceID];
    if(particle.Life <= 0)
        return;

    // Normalized age: 0 at spawn, 1 at death. Life and u_ParticleLifetime share
    // the same (seconds) unit so this ratio is correct. (Sprint 64, task 2.3)
    float t = clamp(1.0 - (particle.Life / u_ParticleLifetime), 0.0, 1.0);
    v_Age = t;

    // Size ramp: small at spawn, PEAK near t=0.3, shrink to zero at death so
    // dying particles vanish rather than pop out at full size (additive blending
    // makes a popping particle read as a flicker artifact). u_Billboard* is the
    // PEAK size. Expect the billboard to snap between integer pixel sizes at
    // 320x180 — that's the pixel-art target, not a bug.
    const float k_PeakAt = 0.3;
    float grow  = smoothstep(0.0, k_PeakAt, t);
    float decay = 1.0 - smoothstep(k_PeakAt, 1.0, t);
    float sizeScale = min(grow, decay);

    float w = u_BillboardWidth  * sizeScale;
    float h = u_BillboardHeight * sizeScale;

    vec2 vertex = Vertices[Indices[gl_VertexID]];
    vec3 cameraRight = vec3(u_View[0][0], u_View[1][0], u_View[2][0]);
    vec3 cameraUp = vec3(u_View[0][1], u_View[1][1], u_View[2][1]);

    vec3 position =
        particle.Position
        + cameraRight * vertex.x * w
        + cameraUp * vertex.y * h;

    gl_Position = u_ViewProj * vec4(position, 1.0);

    v_TexCoords = vertex + 0.5;
}
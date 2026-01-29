#version 330 core

out vec4 FragColor;

uniform vec4  uColor;        // base color (rgb) + intensity (a)
uniform float uTime;         // optional, can be 0.0 if unused
uniform float uGlowStrength; // ~0.6
uniform float uSoftness;     // ~0.15
uniform float uPulse;        // ~0.2 (0 = static)

void main()
{
    // Centered UV
    vec2 uv = gl_PointCoord * 2.0 - 1.0;
    float r = length(uv);

    // Hard cutoff
    if (r > 1.0) discard;

    // Core mask (soft edge)
    float core = smoothstep(1.0, 1.0 - uSoftness, r);

    // Inner glow (brighter toward center)
    float glow = exp(-r * 4.0) * uGlowStrength;

    // Rim highlight (Fresnel-like)
    float rim = pow(1.0 - r, 2.5);

    // Optional subtle pulse (safe if uTime = 0)
    float pulse = 1.0 + sin(uTime * 4.0) * uPulse;

    // Final intensity
    float intensity = (core + glow + rim * 0.35) * pulse;

    // Slight color bloom toward center
    vec3 color = mix(uColor.rgb * 0.85, uColor.rgb * 1.2, glow);

    FragColor = vec4(color, intensity * uColor.a);
}

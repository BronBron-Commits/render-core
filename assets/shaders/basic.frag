#version 330 core

in vec2 vPos;
out vec4 FragColor;

uniform vec4 uColor;
uniform bool uCircle;
uniform bool uGlow;
uniform bool uVignette;
uniform float uTime;

void main()
{
    float alpha = 1.0;

    // circular particles / glow
    if (uCircle || uGlow) {
        float d = length(gl_PointCoord - vec2(0.5));
        alpha = smoothstep(0.5, 0.25, d);
    }

    // vignette darkening
    if (uVignette) {
        float d = length(vPos);
        float vignette = smoothstep(0.4, 1.1, d);
        FragColor = vec4(0.0, 0.0, 0.0, vignette * 0.6);
        return;
    }

    // subtle glow pulse
    if (uGlow) {
        float pulse = 0.85 + 0.15 * sin(uTime * 0.6);
        alpha *= pulse;
    }

    FragColor = vec4(uColor.rgb, uColor.a * alpha);
}

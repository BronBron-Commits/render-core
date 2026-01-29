#version 330 core

in vec2 vLocalPos;
out vec4 FragColor;

uniform vec4  uColor;
uniform float uTime;

void main()
{
    // Very restrained vertical form cue
    float shade =
        1.0
        + smoothstep(-0.05,  0.10, vLocalPos.y) * 0.08   // torso lift
        - smoothstep( 0.15,  0.22, vLocalPos.y) * 0.05   // head settle
        - smoothstep(-0.28, -0.12, vLocalPos.y) * 0.06;  // legs weight

    // Subtle life, strictly bounded
    shade *= 0.97 + 0.03 * sin(uTime * 1.2 + vLocalPos.y * 5.0);

    shade = clamp(shade, 0.90, 1.05);

    FragColor = vec4(uColor.rgb * shade, uColor.a);
}

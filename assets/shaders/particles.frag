#version 330 core

out vec4 FragColor;

uniform vec4 uColor;

void main()
{
    // Circular point sprite
    vec2 uv = gl_PointCoord - vec2(0.5);
    float d = length(uv);

    float mask = smoothstep(0.5, 0.45, d);
    if (mask <= 0.0) discard;

    FragColor = vec4(uColor.rgb, uColor.a * mask);
}

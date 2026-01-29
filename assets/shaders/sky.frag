#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    // vertical gradient
    vec3 top = vec3(0.03, 0.06, 0.12);
    vec3 bottom = vec3(0.01, 0.02, 0.04);
    float t = smoothstep(0.0, 1.0, vUV.y);

    vec3 color = mix(bottom, top, t);

    // subtle moving noise
    float n = hash(vUV * 40.0 + uTime * 0.02);
    color += n * 0.015;

    FragColor = vec4(color, 1.0);
}

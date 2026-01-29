#version 330 core

in vec2 vLocalPos;
out vec4 FragColor;

uniform vec4  uColor;
uniform float uTime;

/* -----------------------------------------
   Strong anatomy shaping
----------------------------------------- */

float verticalForm(float y)
{
    float torso = smoothstep(-0.10,  0.12, y);
    float head  = smoothstep( 0.13,  0.24, y);
    float legs  = smoothstep(-0.34, -0.16, y);

    return
        1.0
        + torso * 0.22
        - head  * 0.12
        - legs  * 0.16;
}

float lateralForm(float x)
{
    float ax = abs(x);
    return 1.0 - smoothstep(0.045, 0.16, ax) * 0.20;
}

float topLight(float y)
{
    return 1.0 + smoothstep(-0.05, 0.25, y) * 0.10;
}

/* -----------------------------------------
   Silhouette metrics
----------------------------------------- */

float rimMask(vec2 p)
{
    float ax = abs(p.x);
    float ay = abs(p.y);

    float side = smoothstep(0.065, 0.11, ax);
    float top  = smoothstep(0.20,  0.33, ay);

    return clamp(side + top, 0.0, 1.0);
}

/* -----------------------------------------
   Glow (edge-biased, capped)
----------------------------------------- */

float rimGlow(vec2 p)
{
    float rim = rimMask(p);

    // soft pulse, slow
    float pulse = 0.85 + 0.15 * sin(uTime * 1.4);

    return rim * pulse;
}

/* -----------------------------------------
   Main
----------------------------------------- */

void main()
{
    float shade = 1.0;

    // Structural shading
    shade *= verticalForm(vLocalPos.y);
    shade *= lateralForm(vLocalPos.x);
    shade *= topLight(vLocalPos.y);

    // Life motion
    shade *= 0.93 + 0.07 * sin(uTime * 1.8 + vLocalPos.y * 7.0);

    // Clamp base shading hard
    shade = clamp(shade, 0.75, 1.12);

    vec3 baseColor = uColor.rgb * shade;

    // Dark silhouette cut (keeps form readable)
    float rim = rimMask(vLocalPos);
    baseColor *= (1.0 - rim * 0.28);

    /* -------- GLOW -------- */

    float glow = rimGlow(vLocalPos);

    // Glow color (cool-biased, tweak freely)
    vec3 glowColor = vec3(0.55, 0.55, 1.0);

    // Intensity cap — THIS is what keeps it sane
    vec3 emissive = glowColor * glow * 0.50;

    // Final color: base + glow (bounded)
    vec3 finalColor = baseColor + emissive;

    FragColor = vec4(finalColor, uColor.a);
}

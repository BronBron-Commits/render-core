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
        + torso * 0.22   // strong chest/core
        - head  * 0.12   // head recedes slightly
        - legs  * 0.16;  // heavy lower mass
}

/* Side thickness exaggeration */
float lateralForm(float x)
{
    float ax = abs(x);
    return 1.0 - smoothstep(0.045, 0.16, ax) * 0.20;
}

/* Top-down light illusion */
float topLight(float y)
{
    return 1.0 + smoothstep(-0.05, 0.25, y) * 0.10;
}

/* -----------------------------------------
   Graphic silhouette
----------------------------------------- */

float rimDarken(vec2 p)
{
    float ax = abs(p.x);
    float ay = abs(p.y);

    float side = smoothstep(0.065, 0.11, ax);
    float top  = smoothstep(0.20,  0.33, ay);

    return clamp(side + top, 0.0, 1.0);
}

/* -----------------------------------------
   Main
----------------------------------------- */

void main()
{
    float shade = 1.0;

    // Shape first
    shade *= verticalForm(vLocalPos.y);
    shade *= lateralForm(vLocalPos.x);
    shade *= topLight(vLocalPos.y);

    // Assertive life motion
    shade *= 0.93 + 0.07 * sin(uTime * 1.8 + vLocalPos.y * 7.0);

    // HARD clamp: prevents any blowout
    shade = clamp(shade, 0.75, 1.12);

    vec3 color = uColor.rgb * shade;

    // Strong silhouette cut (ink-like)
    float rim = rimDarken(vLocalPos);
    color *= (1.0 - rim * 0.28);

    FragColor = vec4(color, uColor.a);
}

#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec2 uOffset;
uniform float uScale;
uniform float uPointSize;

out vec2 vPos;

void main()
{
    vec2 pos = aPos * uScale + uOffset;
    vPos = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
    gl_PointSize = uPointSize;
}

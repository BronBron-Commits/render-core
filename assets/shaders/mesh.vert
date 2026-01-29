#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 uOffset;

out vec2 vLocalPos;

void main()
{
    vLocalPos = aPos;
    vec2 p = aPos + uOffset;
    gl_Position = vec4(p, 0.0, 1.0);
}

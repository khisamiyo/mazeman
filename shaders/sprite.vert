#version 330 core
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

out vec2 uv;

uniform mat4 transform;
uniform float radian;
uniform vec2 origin;
uniform vec4 rect;

vec2 rotate(vec2 v, float a)
{
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, s, -s, c);
    return m * v;
}

vec2 makePosition()
{
    vec2 a = in_position;
    vec2 b = a - (origin * rect.zw);
    vec2 c = b - rect.xy;
    return rotate(c, radian) + rect.xy;
}

void main()
{
    gl_Position = transform * vec4(makePosition(), 0, 1);

    uv = in_uv;
}

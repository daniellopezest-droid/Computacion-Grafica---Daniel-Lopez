#version 330 core

out vec4 colorFragmento;

uniform vec3 uColor;

void main()
{
    colorFragmento = vec4(uColor, 1.0);
}

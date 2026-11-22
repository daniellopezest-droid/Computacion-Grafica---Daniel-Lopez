#version 330 core

layout(location = 0) in vec2 posAtributo;

uniform mat4 uTransformacion;

void main()
{
    gl_Position = uTransformacion * vec4(posAtributo, 0.0, 1.0);
}

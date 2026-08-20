#version 450

layout(location = 0) out vec4 outColor;

layout (location = 1) in vec4 vcol;

void main()
{
    //outColor = vec4(0.8, 0.1, 0.05, 1.0);
    outColor = vcol;
}

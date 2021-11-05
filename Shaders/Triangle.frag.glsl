#version 450

// Output write
layout (location = 0) out vec4 outFragColor;

void main()
{
    // Return hardcoded red color
    outFragColor = vec4(1.f,1.f,1.f,1.0f);
}

#version 400 core

in vec2 vTexCoord;
out vec4 vOutputColour;

uniform sampler2D textSampler;
uniform vec4 textColor;

void main()
{
    vec4 vTexColour = vec4(1.0, 1.0, 1.0, texture(textSampler, vTexCoord).r);
    vOutputColour = textColor * vTexColour;
}

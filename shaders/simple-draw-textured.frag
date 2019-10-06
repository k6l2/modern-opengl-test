#version 140
uniform sampler2D textureSampler2d;
in vec2 fragTexCoord;
in vec4 fragColor;
layout(location = 0) out vec4 outputFragment;
void main()
{
	outputFragment = 
		texture(textureSampler2d, fragTexCoord)*fragColor;
}
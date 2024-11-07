#version 450 core

out vec4 fColor;
in vec4 fragColour;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
	vec4 ambient = vec4(0.1,0.1,0.1,1.0);
	fColor = fragColour;
}

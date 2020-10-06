#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform int xsize;
uniform int ysize;
uniform mat4 mvp;

void main() 
{
	gl_Position = mvp * vPosition;

	color = vColor;	
} 

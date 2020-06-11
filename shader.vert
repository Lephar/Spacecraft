#version 460 core

layout (binding = 0) uniform block
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec4 normal;

out vec4 vertexPosition;
out vec4 vertexColor;
out vec4 vertexNormal;

void main()
{
	gl_Position = projection * view * model * position;
	vertexPosition = model * position;
	vertexColor = color;
	vertexNormal = model * normal;
}

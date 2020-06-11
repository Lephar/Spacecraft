#version 460 core

in vec4 vertexPosition;
in vec4 vertexColor;
in vec4 vertexNormal;

out vec4 fragmentColor;

void main()
{
	vec4 lightColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4 lightPosition = vec4(0.0f, 0.0f, 240.0f, 1.0f);
	vec4 lightDirection = normalize(lightPosition - vertexPosition);
	float intensity = max(dot(vertexNormal, lightDirection), 0.0f);
	vec4 pointLight = intensity * lightColor;
	vec4 ambientLight = vec4(0.2f, 0.2f, 0.2f, 1.0f);

	fragmentColor = (pointLight + ambientLight) * vertexColor;
	//fragmentColor = (vec4(1.0f, 1.0f, 1.0f, 1.0f) + vertexNormal) / 2;
	//fragmentColor = vertexColor;
}

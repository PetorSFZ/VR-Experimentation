#version 330

in vec3 inPosition;
in vec3 inNormal;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix; // inverse(transpose(modelViewMatrix)) for non-uniform scaling

out vec3 pos;
out vec3 normal;

void main()
{
	mat4 modelViewMatrix = uViewMatrix * uModelMatrix;
	pos = (modelViewMatrix * vec4(inPosition, 1.0)).xyz;
	normal = normalize((uNormalMatrix * vec4(inNormal, 0.0)).xyz);
	gl_Position = uProjMatrix * modelViewMatrix * vec4(inPosition, 1.0);
}
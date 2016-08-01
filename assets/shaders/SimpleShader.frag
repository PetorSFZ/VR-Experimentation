#version 330

// Input, output and uniforms
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Input
in vec3 pos;
in vec3 normal;
in vec3 lightPos;

// Uniforms
//uniform float uFarPlaneDist;

// Output
//layout(location = 0) out vec4 outFragLinearDepth;
//layout(location = 1) out vec4 outFragNormal;
out vec4 outFragColor;

// Uniforms

// Main
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void main()
{
	vec3 toLight = lightPos - pos;
	vec3 toLightDir = normalize(toLight);
	outFragColor = vec4(vec3(0.2) + vec3(clamp(dot(toLightDir, normal), 0, 1)), 1.0);

	//outFragLinearDepth = vec4(-pos.z / uFarPlaneDist, 0.0, 0.0, 1.0);
	//outFragNormal = vec4(normal, 1.0);
	//outFragColor = vec4(1,0,0,1);
	//outFragColor = vec4(normal, 1.0);
}
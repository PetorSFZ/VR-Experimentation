#version 330

// Input, output and uniforms
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Input
in vec3 pos;
in vec3 normal;

// Uniforms
//uniform float uFarPlaneDist;

// Output
//layout(location = 0) out vec4 outFragLinearDepth;
//layout(location = 1) out vec4 outFragNormal;
out vec4 outFragColor;

// Main
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void main()
{
	//outFragLinearDepth = vec4(-pos.z / uFarPlaneDist, 0.0, 0.0, 1.0);
	//outFragNormal = vec4(normal, 1.0);
	outFragColor = vec4(1,0,0,1);
}
#version 300 es
//#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out float diffLightIntensity;
out float specLightIntensity;
out float fireBugLightIntensity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float material_shininess;
uniform vec3 firebugLightPos;
const float firebugQuadraticAttenuation = 12.2;

// https://www.khronos.org/registry/OpenGL-Refpages/es1.1/xhtml/glMaterial.xml
// https://www.ics.com/blog/fixed-function-modern-opengl-part-4-4

void main()
{
	vec4 modelPosition = (model * vec4(aPos, 1.0f));
	gl_Position = projection * view * modelPosition;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	//vec3 lightPos = vec3(0, 20, 0);
	//vec3 lightDir = normalize(lightPos - gl_Position);

	vec3 lightDirection = normalize(vec3(0, 1, 0.2));
	vec3 modelNormal = normalize(  model*vec4(aNormal, 0.0f)  ).xyz;
    diffLightIntensity = max(0.0, dot(modelNormal, lightDirection));

	vec3 halfVector = normalize(lightDirection + vec3(0,0,1)); // light half vector  (listpOsition -> lightDirection).
	float nDotVP = diffLightIntensity;
	float nDotHV    = max(0.f, dot(modelNormal,  halfVector));      // normal . light half vector
	float pf        = mix(0.f, pow(nDotHV, material_shininess), step(0.f, nDotVP)); // power factor
	specLightIntensity = pf;

	float lightDistance = distance(firebugLightPos, modelPosition.xyz);
	vec3 fireBugLightDir = normalize(firebugLightPos - modelPosition.xyz);
	fireBugLightIntensity = firebugQuadraticAttenuation * max(0.2, dot(modelNormal, fireBugLightDir)) / (lightDistance * lightDistance);
}
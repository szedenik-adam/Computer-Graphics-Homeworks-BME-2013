#version 300 es
//#version 330 core

precision mediump float;

out vec4 FragColor;

in vec2 TexCoord; // From vertex shader.
in float diffLightIntensity;
in float specLightIntensity;
in float fireBugLightIntensity;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

struct Material {
    vec3  ambient;
    vec3  diffuse;
	vec3  specular;
};
uniform Material material;

void main()
{
	vec4 ambLightColor = vec4(0.9f, 0.82f, 0.32f, 1.f);
	float ambLightIntensity = 0.1f;

	vec4 color, baseColor;
	if(material.ambient == vec3(0,0,0))
	{
		// linearly interpolate between both textures (80% container, 20% awesomeface)
		// vec4 color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
		baseColor = texture(texture1, TexCoord);
		// Apply lighting.
		color = vec4((baseColor * diffLightIntensity + baseColor*ambLightColor*ambLightIntensity).rgb, 1.0);
	}
	else
	{
		baseColor = vec4(material.specular, 0.0f);
		vec3 color3 = material.ambient * ambLightColor.rgb * ambLightIntensity;
		color3 += material.diffuse * diffLightIntensity;
		color3 += material.specular * specLightIntensity;
		color = vec4(color3, 1.0);
	}
	//if(material.ambient != vec3(0.05f, 0.05f, 0.05f)) // If the material is not shadow. (if the shadow is not black, try to replace this with <0.05+epsilon & >0.05-epsilon check!)
	{
		color += baseColor * vec4(vec3(0.4f, 0.35f, 0.1f) * fireBugLightIntensity, 0.0f);
	}
	FragColor = color;
}
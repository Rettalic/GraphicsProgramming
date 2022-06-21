#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;
in vec4 worldPixel;

uniform sampler2D diffuseSampler;

void main() {
	//FragColor = vec4(1, 1, 1, 1);
	vec3 lightDir = normalize(vec3(0, -1, -1));
	vec3 camPos = vec3(0, 3, -3);
	vec3 viewDirection = normalize(worldPixel.xyz - camPos);

	vec3 lightReflect = normalize(reflect(-lightDir, normal));
	float specular = pow(max(dot(lightReflect, viewDirection), 0.0), 128);

	vec4 diffuseColor = texture(diffuseSampler, uv);

	float light = max(dot(-lightDir, normal), .25);

	FragColor = diffuseColor * light + specular * diffuseColor;// vec4(diffuseColor.rgb, 1.0f); //vec4(normal * .5 + .5, 0);// 
}
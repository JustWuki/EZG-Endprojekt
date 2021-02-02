#version 330 core
// based on https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
out vec4 FragColor;

in VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	mat3 TBN;
} fs_in;

uniform sampler2D shadowMap;
uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float bumpiness;

uniform mat4 model;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}

void main()
{
	vec4 baseColor =  texture(albedoTexture,fs_in.TexCoords);
	vec3 norm = texture(normalTexture, fs_in.TexCoords).xyz;

	norm = norm * 2.0 - 1.0;
	norm = normalize(fs_in.TBN * norm);
	norm = mix(norm,fs_in.Normal,min(max(bumpiness,0.0),1.0));
	vec3 lightDir = normalize(lightPos - fs_in.FragPos); 

	// calculate diffuse lighting
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * baseColor.rgb;

	//calculate specular lighting
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
	vec3 specular = 0.5 * spec * vec3(1,1,1); 

	// calculate shadows
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace,norm, lightDir);

	// get total lighting value
	FragColor = (vec4(diffuse, 1.0) + vec4(specular, 1.0)) * (1.0 - shadow) + baseColor * 0.1;
}
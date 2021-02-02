#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

in mat3 TBN;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
  
uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform float bumpiness;

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
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;   
    normal.xy *= bumpiness;
    normal = normalize(TBN * normal); 

    vec3 color = texture(diffuseTexture, TexCoords).rgb;
    vec3 lightColor = vec3(1.0);

    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * color;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor; 

    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; 

    FragColor = vec4(lighting, 1.0);
} 
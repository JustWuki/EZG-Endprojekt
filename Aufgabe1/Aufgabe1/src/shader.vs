#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoords;
out vec4 FragPosLightSpace;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	vec3 N = normalize(transpose(inverse(mat3(model))) * aNormal);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);

	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = transpose(inverse(mat3(model))) * aNormal;
	TexCoords = aTexCoords;
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	gl_Position = projection * view * vec4(FragPos, 1.0);
}
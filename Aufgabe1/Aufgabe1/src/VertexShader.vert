#version 330 core
// based on https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
	vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	vec3 N = normalize(transpose(inverse(mat3(model))) * aNormal);
	vec3 B = cross(N, T);
	vs_out.TBN = mat3(T, B, N);

	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}
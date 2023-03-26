#version 460 core
#define MAX_LIGHTS 16

struct PointLight{
	vec3 position;
	vec3 color;
};

vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 kd = vec3(0.8, 0.8, 0.8);   // diffuse reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
int phongExponent = 400; 

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

uniform int lightCount;
uniform PointLight pointLights[MAX_LIGHTS];

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec4 color;

void main(void){

	vec4 pWorld = modelingMatrix * vec4(inVertex, 1);
	vec3 nWorld = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

	vec3 vP = vec3(pWorld); // Vertex Position
	vec3 V = normalize(eyePos - vP);
	vec3 N = normalize(nWorld);

	vec3 lightPos, I, L, H;
	float NdotL, NdotH;
	float distanceModifier = 0.00001;
	vec3 diffuseColor  = vec3(0,0,0);
	vec3 specularColor = vec3(0,0,0);
	for(int i = 0 ; i<lightCount ; ++i){
		lightPos = pointLights[i].position;	// light position in world coordinates
		distanceModifier += (vP.x - lightPos.x)*(vP.x - lightPos.x) 
						  + (vP.y - lightPos.y)*(vP.y - lightPos.y) 
						  + (vP.z - lightPos.z)*(vP.z - lightPos.z);
		I = pointLights[i].color / distanceModifier ;   		// point light intensity
		L = normalize(lightPos - vec3(pWorld));
		H = normalize(L + V);
		NdotL = dot(N, L); // for diffuse component
		NdotH = dot(N, H); // for specular component
		diffuseColor = (I * kd * max(0, NdotL)) + diffuseColor;
		specularColor = (I * ks * pow(max(0, NdotH), phongExponent)) + specularColor ;
	}

	vec3 ambientColor = Iamb * ka;

	color = vec4(diffuseColor + specularColor + ambientColor, 1);
    	gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
}


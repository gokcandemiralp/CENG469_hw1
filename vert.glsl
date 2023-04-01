#version 460 core
#define MAX_LIGHTS 16

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec4 fragWorldPos;
out vec3 Normal;

void main(void){
    Normal = inNormal;
    fragWorldPos = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
}


#version 450 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

uniform vec3 controlSurfaces[6][6][4][4];
uniform int anchorCountY;
uniform int anchorCountX;
uniform int sampleRate;

layout(location=0) in vec2 inUV;
layout(location=1) in vec2 inAnchor;

out vec4 fragWorldPos;
out vec3 Normal;

void main(void){
    int anchorDownScale = max(anchorCountX,anchorCountY);
    vec3 deneme = controlSurfaces[0][1][2][3];
    float step = 1.0/(sampleRate-1);
    float fraction = step/anchorDownScale;

    //Normal = inNormal;
    fragWorldPos = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inUV.x - 0.5 , inUV.y - 0.5 , 0, 1);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inUV.x - 0.5 , inUV.y - 0.5 , 0, 1);
}


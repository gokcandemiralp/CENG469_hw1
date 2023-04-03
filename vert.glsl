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

vec3 controlPoints[4][4];
float u = inUV.x;
float v = inUV.y;


int Combination(int n, int k){
    if( k==0 || n == k ){return 1;}
    k = min(k , (n-k));
    int divident = n;
    int divisor = k;
    for(; k > 1 ;){
        divident *= --n;
        divisor *= --k;
    }    
    return divident/divisor;
}

float Bernstein(int i, int n, float s){
    if((s == 1 && i==3) || (s ==  0 && i == 0)){return 1;}
    return Combination(n,i)*pow((1-s),(n-i))*pow(s,i);
}

float calcBezierSurface(){
    float ans = 0;
    for(int i = 0 ; i<4 ; ++i){
        for(int j = 0 ; j<4 ; ++j){
            ans = ans + controlPoints[i][j].z * Bernstein(i,3,v) * Bernstein(j,3,u);
        }        
    }
    return ans;
}

vec3 dVBezier() { 
    vec3 ans = vec3(0,0,0);
    for(int i = 0 ; i<4 ; ++i ){
        ans += -3 * (1 - v) * (1 - v) * controlPoints[i][0] * Bernstein(i,3,u) + 
                (3 * (1 - v) * (1 - v) - 6 * v * (1 - v)) * controlPoints[i][1] * Bernstein(i,3,u) + 
                (6 * v * (1 - v) - 3 * v * v) * controlPoints[i][2] * Bernstein(i,3,u) + 
                3 * v * v * controlPoints[i][3] * Bernstein(i,3,u); 
    }
    return ans;
} 
 
vec3 dUBezier() { 
    vec3 ans = vec3(0,0,0);
    for(int i = 0 ; i<4 ; ++i ){
        ans += -3 * (1 - u) * (1 - u) * controlPoints[0][i] * Bernstein(i,3,v) + 
                (3 * (1 - u) * (1 - u) - 6 * u * (1 - u)) * controlPoints[1][i] * Bernstein(i,3,v) + 
                (6 * u * (1 - u) - 3 * u * u) * controlPoints[2][i] * Bernstein(i,3,v) + 
                3 * u * u * controlPoints[3][i] * Bernstein(i,3,v); 
    }
    return ans;
} 

vec3 calcBezierNormal(){
    vec3 dU = dUBezier(); 
    vec3 dV = dVBezier();
    return normalize(cross(dV,dU));
}

void main(void){
    float tempZ = 0;
    int anchorDownScale = max(anchorCountX,anchorCountY);
    float step = 1.0/(sampleRate-1);
    float fraction = step/anchorDownScale;
    float surfaceSize = 1.0/anchorDownScale;
    controlPoints = controlSurfaces[0][0];

    tempZ = calcBezierSurface();
    Normal = calcBezierNormal();
    // glm::vec3 normalVector = calcBezierNormal(step*iy, step*ix, controlSurfaces[anchorY][anchorX]);
    vec4 modelPosition = vec4(u/anchorDownScale + (surfaceSize*inAnchor.x) - 0.5 , v/anchorDownScale + (surfaceSize*inAnchor.y)- 0.5 , tempZ, 1);
    fragWorldPos = projectionMatrix * viewingMatrix * modelingMatrix * modelPosition;
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * modelPosition;
}


#include "utils.h"

//gVertices.push_back(Vertex(c1, c2, c3));
//gNormals.push_back(Normal(c1, c2, c3));
//gFaces.push_back(Face(vIntIndexdex, tIndex, nIndex));

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
    return Combination(n,i)*pow((1-s),(n-i))*pow(s,i);
}

float BernsteinD(int i, int n, float s){
    return Combination(n,i)*pow((1-s),(n-i))*pow(s,i);
}

float calcBezierSurface(float s, float t, glm::vec3 (&controlPoints)[4][4]){
    float ans = 0;
    for(int i = 0 ; i<4 ; ++i){
        for(int j = 0 ; j<4 ; ++j){
            ans = ans + controlPoints[i][j].z * Bernstein(i,3,s) * Bernstein(j,3,t);
        }        
    }
    return ans;
}
 

glm::vec3 dUBezier(const float &u, const float &v, glm::vec3 (&controlPoints)[4][4]) { 

    glm::vec3 ans = glm::vec3(0,0,0);
    for(int i = 0 ; i<4 ; ++i ){
        ans += -3 * (1 - v) * (1 - v) * controlPoints[i][0] * Bernstein(i,3,u) + 
                (3 * (1 - v) * (1 - v) - 6 * v * (1 - v)) * controlPoints[i][1] * Bernstein(i,3,u) + 
                (6 * v * (1 - v) - 3 * v * v) * controlPoints[i][2] * Bernstein(i,3,u) + 
                3 * v * v * controlPoints[i][3] * Bernstein(i,3,u); 
    }
    return ans;
} 
 
glm::vec3 dVBezier(float u, float v, glm::vec3 (&controlPoints)[4][4]) { 

    glm::vec3 ans = glm::vec3(0,0,0);
    for(int i = 0 ; i<4 ; ++i ){
        ans += -3 * (1 - u) * (1 - u) * controlPoints[0][i] * Bernstein(i,3,v) + 
                (3 * (1 - u) * (1 - u) - 6 * u * (1 - u)) * controlPoints[1][i] * Bernstein(i,3,v) + 
                (6 * u * (1 - u) - 3 * u * u) * controlPoints[2][i] * Bernstein(i,3,v) + 
                3 * u * u * controlPoints[3][i] * Bernstein(i,3,v); 
    }
    return ans;
} 

glm::vec3 calcBezierNormal(float u, float v, glm::vec3 (&controlPoints)[4][4]){
    glm::vec3 dU = dUBezier(u, v, controlPoints); 
    glm::vec3 dV = dVBezier(u, v, controlPoints);
    return normalize(cross(dU,dV));
}

void PrintControlPoints(int cPointY,int cPointX, float **controlPoints){
    for(int iy = 0; iy < cPointY ; ++iy){
        for(int ix = 0; ix < cPointX ; ++ix){
            float tempVertex = controlPoints[iy][ix];
            cout << "[" << tempVertex << "]";
        }
        cout << "\n";
    }
}
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

float calcBezierSurface(float s, float t, Vertex **controlPoints){
    float ans = 0;
    for(int i = 0 ; i<4 ; ++i){
        for(int j = 0 ; j<4 ; ++j){
            ans = ans + controlPoints[i][j].z * Bernstein(i,3,s) * Bernstein(j,3,t);
        }        
    }
    return ans;
}

glm::vec3 calcBezierNormal(float s, float t, Vertex **controlPoints){
    glm::vec3 ans;
    for(int i = 0 ; i<3 ; ++i){
        for(int j = 0 ; j<3 ; ++j){
            ans.x -= controlPoints[i][j].x-controlPoints[i+1][j+1].x * 9 * Bernstein(i,2,s) * Bernstein(j,2,t);
            ans.y -= controlPoints[i][j].y-controlPoints[i+1][j+1].y * 9 * Bernstein(i,2,s) * Bernstein(j,2,t);
            ans.z -= controlPoints[i][j].z-controlPoints[i+1][j+1].z * 9 * Bernstein(i,2,s) * Bernstein(j,2,t);
        }        
    }
    return ans;
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
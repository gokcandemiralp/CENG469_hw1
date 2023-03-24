#include "utils.h"

float Combination(int n, int k){
    float value = 1.0;
    for (int i = 1; i <= k; i++){
        value = value * ((n + 1 - i) / i);
    }
    if (n == k){
        value = 1;
    }
    return value;
}

float Bernstein(int i, int n, float s){
    return Combination(n,i)*pow((1-s),(n-i))*pow(s,i);
}

Vertex BezierSurface(int n, int m, float s, float t, Vertex **controlPoints){
    Vertex ans;
    for(int i = 0 ; i<n ; ++i){
        for(int j = 0 ; j<m ; ++j){
            ans = ans + controlPoints[i][j] * Bernstein(i,n,s) * Bernstein(j,m,t);
        }        
    }
    return ans;
}

void PrintControlPoints(int cPointY,int cPointX, Vertex **controlPoints){
    for(int iy = 0; iy < cPointY ; ++iy){
        for(int ix = 0; ix < cPointX ; ++ix){
            Vertex tempVertex = controlPoints[iy][ix];
            tempVertex.printVertex();
        }
        cout << "\n";
    }
}
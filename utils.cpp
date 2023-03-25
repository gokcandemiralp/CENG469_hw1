#include "utils.h"

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
    cout << "n:" << n << " i:" << i << " = " << Combination(n,i) << "\n";
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
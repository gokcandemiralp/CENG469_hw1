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
    // cout << "n:" << n << " i:" << i << " = " << Combination(n,i) << "\n";
    // cout << "Combination( " << n << "," << i << ") = " << Combination(n,i) 
    //      << " | pow(" << 1-s << "," << n-i << ")) = " << pow((1-s),(n-i)) 
    //      << " | pow(" << s << "," << i << ") = " << pow(s,i) << "\n";
    return Combination(n,i)*pow((1-s),(n-i))*pow(s,i);
}


float calcBezierSurface(float s, float t, float **controlPoints){
    float ans = 0;
    for(int i = 0 ; i<4 ; ++i){
        for(int j = 0 ; j<4 ; ++j){
            // cout << "ans["<< i <<"," << j <<"] = " <<  Bernstein(i,n-1,s) * Bernstein(j,m-1,t) << "\n";
            ans = ans + controlPoints[i][j] * Bernstein(i,3,s) * Bernstein(j,3,t);
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
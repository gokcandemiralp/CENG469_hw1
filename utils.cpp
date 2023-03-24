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

void BezierSurface(int n, int m, float s, float t){
    for(int i = 0 ; i<n ; ++i){
        for(int j = 0 ; j<m ; ++j){
            Bernstein(i,n,s) * Bernstein(j,m,t);
        }        
    }
}
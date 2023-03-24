#include <utils.h>

float combination(int n, int k){
    float value = 1.0;
    for (int i = 1; i <= k; i++){
        value = value * ((n + 1 - i) / i);
    }
    if (n == k){
        value = 1;
    }
    return value;
}
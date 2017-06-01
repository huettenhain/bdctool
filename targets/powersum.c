#include <stdio.h>
#include "../target.h"

const int DEGREE = 4;
const int DIMENSION = 3;

void __print_constant(FILE* f, char c) {
    fprintf(f,"%c",c); }

void __print_variable(FILE* f, int v)  {
    switch(v) {
        case 0: fprintf(f,"x"); break;
        case 1: fprintf(f,"y"); break;
        case 2: fprintf(f,"z"); break;
        default: break;
    }
}

entry __target(entry a[DIMENSION]) {
    entry s=0;
    for (int i=0; i<DIMENSION; i++)
    s = R(s+Power(Subscript(a,i),DEGREE));
    return s;
}
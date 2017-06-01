#include <stdio.h>
#include "../target.h"

const int DEGREE = 6;
const int DIMENSION = (DEGREE+3);

void __print_constant(FILE* f, char c) {
    fprintf(f,"%c",c); 
}

void __print_variable(FILE* f, int v)  {
    switch(v) {
        case  0: fprintf(f,"x"); break;
        case  1: fprintf(f,"y"); break;
        default: fprintf(f,"Subscript[a,%d]",v-2);
    }
}

entry __target(entry a[DIMENSION]) {
    entry rv = 0;
    for (int i=0; i<=DEGREE; i++)
        rv = rv + R(a[i+2]*R(Power(a[0],i)*Power(a[1],DEGREE-i)));
    return R(rv);
}
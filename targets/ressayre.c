#include <stdio.h>
#include "../target.h"

const int DEGREE = 3;
const int DIMENSION = 4;

const char VARIABLES[4] = {'x','y','z','t'};

void __print_constant(FILE* f, char c) {
    fprintf(f,"%c",c); }

void __print_variable(FILE* f, int v)  {
    fprintf(f,"%c",VARIABLES[v]);
}

entry __target(entry a[DIMENSION]) {
    entry x = a[0],
          y = a[1],
          z = a[2],
          t = a[3];
    return R(R(x*R(y*y))+R(y*R(t*t))+R(z*R(z*z)));
}
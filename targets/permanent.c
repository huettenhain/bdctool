#include <stdio.h>
#include "../target.h"

static const int DEGREE = 3;
static const int DIMENSION = 9;

const char *VARIABLES[9]
  = {"x11", "x12", "x13", "x21", "x22", "x23", "x31", "x32", "x33"};

entry __target(entry a[9]) {
    return R(
         R(R(a[0]*a[4])*a[8])+R(R(a[0]*a[5])*a[7])
        +R(R(a[1]*a[3])*a[8])+R(R(a[1]*a[5])*a[6])
        +R(R(a[2]*a[3])*a[7])+R(R(a[2]*a[4])*a[6]));
}

void __print_constant(FILE* f, char c) {
    fprintf(f,"  %c",c); 
}
    
void __print_variable(FILE* f, int v)  {
    fprintf(f,VARIABLES[v]);
}
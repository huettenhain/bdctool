#include <stdio.h>
#include "../target.h"

const int DEGREE = 3;
const int DIMENSION = 9;

const char *VARIABLES[9]
 = {"x11", "x12", "x13", "x21", "x22", "x23", "x31", "x32", "x33"};

entry __target(entry a[DIMENSION]) {
    return R(R(R(Subscript(a,0)*Subscript(a,1))*Subscript(a,3))
            + R(R(Subscript(a,1)*Subscript(a,3))*Subscript(a,4))
            + R(R(Subscript(a,0)*Subscript(a,2))*Subscript(a,6))
            + R(R(Subscript(a,1)*Subscript(a,5))*Subscript(a,6))
            + R(R(Subscript(a,2)*Subscript(a,3))*Subscript(a,7))
            + R(R(Subscript(a,4)*Subscript(a,5))*Subscript(a,7))
            + R(R(Subscript(a,2)*Subscript(a,6))*Subscript(a,8))
            + R(R(Subscript(a,5)*Subscript(a,7))*Subscript(a,8))
            + Power(Subscript(a,0),3) + Power(Subscript(a,4),3)
            + Power(Subscript(a,8),3));
}

void __print_constant(FILE* f, char c) {
    fprintf(f,"  %c",c); 
}
    
void __print_variable(FILE* f, int v)  {
    fprintf(f,VARIABLES[v]);
}
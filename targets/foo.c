#include <stdio.h>
#include "../target.h"

const int DEGREE = 2;     // the degree of foo is 2
const int DIMENSION = 3;  // foo uses 3 variables

entry __target(entry a[3]) {
    entry x = a[0], // the array a contains unsigned integers
          y = a[1], // which are placeholders for the variables.
          z = a[2]; 

/*  the macro R is reduction modulo a large prime number.
    instead of actually implementing polynomial arithmetic,
    the variables are replaced by random elements of a 
    large finite field. */

    return R( R(x*y) + z );
}

void __print_constant(FILE* f, char c) {
    fputc(c,f);
}
    
void __print_variable(FILE* f, int v)  {
    static char vv[3] = { 'x', 'y', 'z' };
    fputc(vv[v],f);
}
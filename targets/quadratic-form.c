#include <stdio.h>
#include "../target.h"

const int FORMDIM = 4;

const int DEGREE = 3;
const int DIMENSION = (((FORMDIM)*(FORMDIM)+3*(FORMDIM))/2);

void __print_constant(FILE* f, char c) {
    fprintf(f,"%2c",c); }

void __print_variable(FILE* f, int v)  {
    if (v < FORMDIM) {
        //fprintf(f,"Subscript[x,%i]",v);
          fprintf(f,"x%i",v);
    } else {
        //fprintf(f,"Subscript[a,%i]",v-FORMDIM);
          fprintf(f,"a%i",v-FORMDIM);
    }
}
entry __target(entry v[DIMENSION]) {
    entry rv = 0;
    int i, j, k;
    for (i=k=0; i<FORMDIM; i++) {
        for (j=i; j<FORMDIM; j++) {
            ASSERT(FORMDIM + k < DIMENSION);
            rv += R(v[FORMDIM+(k++)]*R(v[i]*v[j]));
        }
    }
    return R(rv);
}
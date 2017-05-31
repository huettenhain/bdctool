#ifndef _TARGET_H
#define _TARGET_H

#include "finitefield.h"

#define Subscript(a,i) a[i]
#define Power field_pow

extern int const DEGREE;
extern int const DIMENSION;

void __print_constant(FILE* f, char c);
void __print_variable(FILE* f, int v);

entry __target(entry v[DIMENSION]);

#endif 
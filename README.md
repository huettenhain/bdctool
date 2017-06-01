# Binary Determinantal Expression Tool

In the article [Binary Determinantal Complexity](http://dx.doi.org/10.1016/j.laa.2016.04.027) (also available as [arXiv:1410.8202](https://arxiv.org/abs/1410.8202)), we prove that for every integer polynomial *F*&nbsp;∈&nbsp;**ℤ**[*x*<sub>1</sub>,…,*x*<sub>*n*</sub>], there is a matrix *A*&nbsp;∈&nbsp;{0,1,*x*<sub>1</sub>,…,*x*<sub>*n*</sub>}<sup>*d*×*d*</sup> (called a **binary variable matrix**) with det(*A*)=*F*. This software can be used together with the software [nauty](http://pallini.di.uniroma1.it/) to compute the matrix *A* from a given *F*. The tool performs a reasonable, but still slow branch and bound search on all possible variable patterns that are possible for a given matrix *B*&nbsp;∈&nbsp;{0,1}<sup>*d*×*d*</sup>. To find an appropriate matrix *B*, it is sufficient to search the set of all matrices defining bipartite graphs on *d*+*d* vertices. For more information on the procedure, please refer to the article itself.

## How to compile

### Running Make
The project has a makefile with the following targets:
* `debug` (compile with debug information)
* `release` (compile without debug information)
* `openmp` (compile with `-fopenmp` for parallelization)
* `clean` (the usual)

So in order to simply build `bdctool`, run `make release` or `make openmp` if you want parallelization.

### Different Polynomials
By default, bdctool will search for binary variable matrices *A* which satisfiy det(*A*)=per<sub>3</sub>. You can search for a different polynomial `foo` by programming its evaluation routine and placing a file called `foo.c` in the `targets` folder, then compile the project with
```bash
$ make release TARGET=foo
```
Assuming `foo` is the polynomial *xy*+*z*&nbsp;∈&nbsp;**ℤ**[*x*,*y*,*z*], the file [`foo.c`](targets/foo.c) should contain the following (see also [`target.h`](target.h)):
```c
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

/* This functions defines how the constants 0 and 1 are written 
   to the output when a matrix representation is found. This is
   only for pretty-printing. */
void __print_constant(FILE* f, char c) {
    fputc(c,f);
}

/* This function defines how variables are written to the output
   when a matrix representation is found. Again, only for pretty
   printing. */
void __print_variable(FILE* f, int v)  {
    static char vv[3] = { 'x', 'y', 'z' };
    fputc(vv[v],f);
}
```
and with this target, you might compile
```bash
$ make release TARGET=foo
gcc -std=c99 -pedantic -Wall -Wextra -o bdctool *.c ./targets/foo.c -O2

$ genbg -q -z 3 3 | ./bdctool.exe
{{ x, 0, z },
 { 1, y, 0 },
 { 0, 1, 1 }}
```
The file [`foo.c`](targets/foo.c) has been added to the targets directory for reference.

## How to verify the results of the paper

After compiling the bdctool with `make release` or `make openmp`, you can verify that there is no 6×6 binary variable matrix whose determinant is the 3×3 permanent by running
```bash
$ if [[ -z $(genbg -q -d2:2 -z 6 6 | ./bdctool) ]]
> then echo 'bdc(per3) > 6';
> fi
bdc(per3) > 6
```
To find the 7×7 Grenet construction for per<sub>3</sub>, run:
```bash
$ genbg -q -d2:2 -D3:3 -z 7 7 | ./bdctool
{{ x11, x12, x13,   0,   0,   0,   0 },
 {   1,   0,   0, x32, x33,   0,   0 },
 {   0,   1,   0, x31,   0, x33,   0 },
 {   0,   0,   1,   0, x31, x32,   0 },
 {   0,   0,   0,   1,   0,   0, x23 },
 {   0,   0,   0,   0,   1,   0, x22 },
 {   0,   0,   0,   0,   0,   1, x21 }}
```

## Limits of the Method

It should be noted that the method is an exponential time search on all possible ways to insert variables in places where a given binary matrix has the entry 1. With substantial effort and time, you can manage to iterate over all 8×8 binary matrices this way, always also depending on the polynomial. I would guess that 9×9 is out of reach, or only possible with a large cluster and a lot of parallelization. 
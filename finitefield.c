#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "myassert.h"
#include "finitefield.h"

typedef struct {
    entry s;
    entry t;
} bezout_t;


bezout_t exeucl(entry a, entry b) {
    bezout_t retval;
    if (!b) {
        retval.s = 1;
        retval.t = 0;
    } else {
        entry q = a/b, r = a%b;
        bezout_t x = exeucl(b,r);
        retval.s = (x.t);
        retval.t = (x.s) - q*(x.t);
    }
    return retval;
}

entry field_reduce( entry a ) {
    if ((a = R(a))<0) a += MODULUS;
    return a;
}

entry _compute_inverse(entry a) {
    bezout_t B = exeucl(MODULUS,a);
    ASSERT(field_reduce(field_reduce(B.t) * a) == 1);
    return field_reduce(B.t);
}

entry field_invert(entry a) {
#if MODULUS < 0x100000000
#if MODULUS < 0x10000
    static uint16_t inverses[MODULUS] = { 0 };
#else
    static uint32_t inverses[MODULUS] = { 0 };
#endif
    if (!inverses[a]) inverses[a] = _compute_inverse(a);
    return inverses[a];
#else
    return _compute_inverse(a);
#endif
}

entry field_pow( entry a, int n ) {
    if (n < 0)
        return field_pow(field_invert(a),-n);
    else switch(n) {
        case  0: return 1;
        case  1: return a;
        default: {
            entry b = field_pow(R(a*a),n/2);
            return (n%2) ? R(a*b) : b;
        }
    }
}

void matrix_print( entry **a, int n ) {
    int i,j;
    for (i=0; i<n; i++) {
        printf("[");
        for (j=0;j<n;j++)
            printf("%3lld ", a[i][j]);
        printf("]\n");
    }
}

void matrix_cpy( entry **b, entry **a, int n ) {
    memcpy( b + n, a + n, n*n*sizeof(**a) );
}

void matrix_zero( entry **a, int n ) {
    memset( a + n, 0, n*n*sizeof(**a) );
}

entry matrix_det( entry** a, int n ) {
    entry d, **b = matrix_create( n );
    matrix_cpy(b,a,n);
    d = _matrix_det( b, n );
    free(b);
    return d;
}

entry _matrix_det( entry** a, int n ) {
    int k,i,j;
    entry det = 1;

    for (k=0; k<n; k++) {
        entry minus_inverse;

        if (a[k][k] == 0) {
            for (i=k+1; i<n; i++) { /* find a nonzero entry below this one */
                if (a[i][k] != 0) {
                    entry t;
                    for (j=k;j<n;j++) {
                        t = a[i][j];
                        a[i][j] = a[k][j];
                        a[k][j] = t;
                    }
                    det = MODULUS - det;
                    goto work;
                }
            }
            return 0;
        }

work:   minus_inverse = MODULUS - field_invert(a[k][k]);

        for (i=k+1;i<n;i++) {
            if (a[i][k]) {
                entry submul = R(minus_inverse * a[i][k]);
                a[i][k] = 0;
                for (j=k+1;j<n;j++)
                    a[i][j] = R(a[i][j] + submul * a[k][j]);
            }
        }
        det = R(det * a[k][k]);
    }
    return det;
}

entry **matrix_create( int n ) {
    int i;
    entry **a = malloc( n*n*sizeof(**a) + n*sizeof(*a) );
    for (i = 0; i < n; i++)
        a[i] = ((entry*) (a + n)) + n * i;
    return a;
}

void matrix_from_f2( entry **a, bitstring *b, int n ) {
    int i, j;
    for (i=0; i<n; i++)
        for (j=0; j<n; j++)
            a[i][j] = f2_entry(b,i,j);
}



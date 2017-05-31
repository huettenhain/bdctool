#include "binmatrix.h"
typedef long long int entry;

#if 0 && !defined(MODULUS)
#define MODULUS 31
#endif
#if 0 && !defined(MODULUS)
#define MODULUS 907
#endif
#if 1 && !defined(MODULUS)
#define MODULUS 0xFFEF
#endif
#if 1 && !defined(MODULUS)
#define MODULUS 0xFFFFFD
#endif
#if 0 && !defined(MODULUS)
#define MODULUS 0xFFFFFFBF
#endif


entry   field_invert  ( entry a        );
entry   field_pow     ( entry a, int n );
entry   field_reduce  ( entry a );

#if 0
entry R(entry a) {
    return a%MODULUS;
}
#else
#define R(a) ((a)%MODULUS)
#define RR R
#endif

void    matrix_cpy    ( entry **b, entry **a, int n );
void    matrix_zero   ( entry **a,            int n );

void    matrix_print  ( entry **a, int n );
entry  _matrix_det    ( entry **a, int n );
entry   matrix_det    ( entry **a, int n );
entry   matrix_per    ( entry **a, int n );
entry **matrix_create (            int n );
void    matrix_from_f2( entry **a, bitstring *b, int n );


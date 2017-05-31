#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binmatrix.h"
#include "myassert.h"

void _f2r6( unsigned char *data, bitstring *b, int n ) {
    int i,j,topbit;
    unsigned char *topbyte;
    topbit = n * (n-1) / 2;
    for (topbyte=data; topbit >= 6; topbyte++)
        topbit -= 6;
    topbit = 6-topbit-1;
    memset(b,0,n*sizeof(*b));
    for (j=0; j<n; j++) {
        for (i=0; i<j+n; i++) {
            ASSERT(*topbyte);
            if (i<n) b[i] |= ((((*topbyte-63) >> topbit) & 1) << j );

            if (!topbit) topbyte++,topbit=5;
            else topbit--;


        }
    }
}

bitstring *f2_create(int n) {
    return calloc(n,sizeof(bitstring));
}

bitstring *f2_read6(unsigned char *data, int *n) {
    bitstring *b;
    *n = ((*data++)-63)/2;
    if ( (b = calloc(*n,sizeof(*b))) )
        _f2r6( data, b, *n );
    return b;
}

int f2_read6_raw( unsigned char *data, bitstring *b, int *n, int max ) {
    *n = ((*data++)-63)/2;
    if ( (*n) > max ) return 0;
    _f2r6( data, b, *n );
    return 1;
}

int f2_entry(bitstring *b, int i, int j) {
    return ((b[i]>>j)&1);
}


void f2_print( bitstring* b, int n ) {
    int i,j;
    for (i=0; i<n; i++) {
        printf("[");
        for (j=0;j<n;j++)
            printf("%s", f2_entry(b,i,j)?"1 ":"0 ");
        printf("]\n");
    }
}

int f2_det( bitstring* b, int n ) {
    int k,i;
    for (k=0; k<n; k++) {
        if (f2_entry(b,k,k) == 0) {
            for (i=k+1; i<n; i++) {
                if (f2_entry(b,i,k)) {
                    bitstring t=b[i];
                    b[i]=b[k];
                    b[k]=t;
                    goto work;
            } }
            return 0;
        }
work:   for (i=k+1;i<n;i++)
            if (f2_entry(b,i,k)) b[i] ^= b[k];
    }
    return 1;
}


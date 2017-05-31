#ifndef _BINMATRIX_H
#define _BINMATRIX_H

typedef unsigned long int bitstring;

int        f2_read6_raw( unsigned char *data, bitstring *b, int *n, int max );
bitstring *f2_read6    ( unsigned char *data,               int *n );
bitstring *f2_create   (                                    int  n );

int        f2_entry ( bitstring *b, int i, int j );
void       f2_print ( bitstring* b, int n );
int        f2_det   ( bitstring* b, int n );

#endif
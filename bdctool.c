/******************************************************************************
 compile:
 gcc -O3 --std=c99 -o bdctool bdctool.c finitefield.c binmatrix.c

 to find the Grenet construction:
 genbg -d2:2 -D3:3 -z 7 7 | ./bdctool

 to find all 7x7 binary variable matrices A that have det(A)=per:
 genbg -z 7 7 | ./bdctool

 ... but this takes too long. We instead split the output of genbg -z 7 7
 into chunks of the form genbg7x7.001, genbg7x7.002, etcetera. You can then
 use
 ./bdctool genbg7x7.001

 to find all appropriate binary variable matrices that come from graphs in the
 file genbg7x7.001.
 *******************************************************************************/

// print progress output
//#define PRINT_PROGRESS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <omp.h>

#include "finitefield.h"
#include "myassert.h"

// magic values for dynamic memory allocation
#define MEM_BASE 20
#define MEM_INC(_m) (2*(_m))
// maximum size of a determinant we will get
#define MAX_DET_SIZE 8*sizeof(bitstring)

/* enable to use every variable at most SUPPORT_LIMIOT times inside the 
 binary variable matrix. */
#ifdef SUPPORT_LIMIT
#define STEST(_s) ((_s) <= (SUPPORT_LIMIT))
#else
#define STEST(_s) 1
#endif

#ifdef EBUG
#define DEBUG_PRINTF(...) do{printf(__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DEBUG_PRINTF(...) ((void)0)
#endif

#if defined( PRINT_PROGRESS )
#define PROGRESS_INIT unsigned int __counter=0
#define PROGRESS_STEP ++__counter
#define PROGRESS_ECHO(input) printf("* operating on graph [%s] which is #%d\n", (input), __counter)
#else
#define PROGRESS_INIT
#define PROGRESS_STEP
#define PROGRESS_ECHO(input)
#endif


/* Given the size of a binary support matrix, this calculates how many of the 
 ones in the matrix can not be replaced by a variable. Since the degree of
 the n x n determinant is n and the degree of the target function is DEGREE,
 we need to have at least n-DEGREE many ones in the matrix to guarantee that
 some monomial has degree at most DEGREE. */
#define PLACEONES(n) ((n)-DEGREE)

#include "target.h"

typedef struct {
    short int row;
    short int col;
} coordinate_t;

typedef enum {
    false = 0, true = 1
} boolean;

typedef struct {
    int count;
    int alloc;
    bitstring *entry;
} candidate_t;

typedef struct {
    int cached;
    candidate_t *size;
} grouping_t;

typedef entry (*TARGET_FUNCTION)(entry[DIMENSION]);

#define GROUPING(d, v) (d)->groupings[v]

typedef struct {
    entry ** source;
    entry **_source;
    entry all_ones_value;
    int candidate_size;
    entry *base;
    entry *variables; /* finite field entries for the 'variables' */
    coordinate_t *support; /* position of slots for variables */
    int support_size;  /* number of slots in this matrix */
    int support_alloc; /* number of slots already allocated */
    int support_freeslots; /* number of slots that can contain variables */
    
    /* groupings[v] has all possible subsets of the support which could a
     priori be set to variable v. */
    grouping_t *groupings;

    // For each variable, the highest index of the variable with the same
    // groupings as v.
    int *groupie;

    int maxplaced;
    TARGET_FUNCTION target;
    int *abort;
} data;

entry eval_target(data *d) {
    return (*d->target)(d->base);
}

entry eval_source(data *d) {
    matrix_cpy(d->_source, d->source, d->candidate_size);
    return _matrix_det(d->_source, d->candidate_size);
}

void matrix_data_print(data *d) {
    int i, j, v, n = d->candidate_size;
    for (i = 0; i < n; i++) {
        printf(i ? " " : "{");
        printf("{ ");
        for (j = 0; j < n; j++) {
            if (d->source[i][j] == 0) {
                __print_constant(stdout,'0');
            } else if (d->source[i][j] == 1) {
                __print_constant(stdout,'1');
            } else {
                for (v = 0; v < DIMENSION; v++) {
                    if (d->source[i][j] == d->base[v]) {
                        __print_variable(stdout,v);
                        goto matrix_print_donehere;
                    }
                }
                __print_constant(stdout,'?');
            }
            matrix_print_donehere: printf((j + 1 == n) ? " }" : ", ");
            ;
        }
        printf((i + 1) != n ? ",\n" : "}\n");
    }
}

void data_free(data *d) {
    for (int v = 0; v < DIMENSION; v++) {
        int s;
        if (!GROUPING(d,v).size)
            continue;
        for (s = 0; s <= GROUPING(d,v).cached; s++) {
            free(GROUPING(d,v).size[s].entry);
        }
        free(GROUPING(d,v).size);
        GROUPING(d,v).size = NULL;
        GROUPING(d,v).cached = 0;
    }
    free(d->source);
    free(d->_source);
    free(d->groupie);
    free(d->groupings);
    free(d->variables);
    free(d->base);

    memset(d, 0, sizeof(*d));
}

/* compute the next bit permutation with the same number of bits set to 1 */
bitstring bs_next(bitstring b) {
    bitstring t = (b | (b - 1)) + 1;
    return t | ((((t & -t) / (b & -b)) >> 1) - 1);
}
/* wrapper functions for better overview */
bitstring bs_shift(int size) {
    return (1 << size);
}
int bs_test(bitstring b, int i) {
    return ((b >> i) & 1);
}

/* count the number of bits that are set in b */
int bs_countbits(bitstring b) {
    int c;
    for (c = 0; b; c++)
      b &= b - 1;
    return c;
}

/* Evaluated for every matrix whose determinant evaluates to the target. Return
 * value indicates how the recursion proceeds:
 *  return 1: Stop recursion
 *  return 0: Keep iterating  */
int the_callback(data *d) {
#ifdef _OPENMP
#pragma omp critical
#endif
    { matrix_data_print(d); }
    return 1;
}

int lazy_groupings(data *d, int v) {
    entry value_target;
    int memory = MEM_BASE;
    entry testbase[DIMENSION];
    bitstring b;
    int i, j, k, c, n = d->candidate_size;
    int s = GROUPING(d,v).cached + 1;

    ASSERT( s <= d->support_freeslots );

    for (i = 0; i < DIMENSION; i++)
        testbase[i] = 1;
    testbase[v] = d->variables[v];
    value_target = (*d->target)(testbase);

    if (!(GROUPING(d,v).size[s].entry = calloc(memory, sizeof(bitstring)))) {
        DEBUG_PRINTF("! failed to allocate array for groupings of variable %d, size %d.\n", v, s);
        return 0;
    }

    c = GROUPING(d,v).size[s].count = 0;

    for (b = bs_shift(s) - 1; !bs_test(b, d->support_size); b = bs_next(b)) {
        matrix_cpy(d->_source, d->source, n);
        for (k = 0; k < d->support_size; k++) {
            i = d->support[k].row, j = d->support[k].col;
            if (bs_test(b, k)) {
                d->_source[i][j] = d->variables[v];
            } else {
                d->_source[i][j] = 1;
            }
        }
        if (_matrix_det(d->_source, n) == value_target) {
            GROUPING(d,v).size[s].entry[c++] = b;
            if (c >= memory) {
                bitstring *tmp;
                if ((tmp = realloc(GROUPING(d,v).size[s].entry, MEM_INC(memory) * sizeof(bitstring)))) {
                    memory = MEM_INC(memory);
                    GROUPING(d,v).size[s].entry = tmp;
                } else {
                    DEBUG_PRINTF("! failed to realloc groupings of size %d from %d to %d memory.\n", s, memory, MEM_INC(memory));
                    free(GROUPING(d,v).size[s].entry);
                    return 0;
                }
            }
        }
    }

    GROUPING(d,v).size[s].entry = realloc(GROUPING(d,v).size[s].entry,
            c * sizeof(bitstring));
    GROUPING(d,v).size[s].count = c;
    GROUPING(d,v).cached = s;

    return 1;
}

int data_alloc(data *d, TARGET_FUNCTION t, int n) {
    int v;
    memset(d, 0, sizeof(*d));
    d->target = t;
    d->candidate_size = n;


    if (
            !(d->base = calloc(DIMENSION,sizeof(*d->base)))
         || !(d->variables = calloc(DIMENSION,sizeof(*d->variables)))
         || !(d->groupings = calloc(DIMENSION,sizeof(*d->groupings)))
         || !(d->groupie = calloc(DIMENSION,sizeof(*d->groupie)))
         || !(d->source  = matrix_create(n))
         || !(d->_source = matrix_create(n)) 
    ){
        DEBUG_PRINTF("! failed to allocate memory.\n");
        data_free(d);
        return 0;
    }


    for (v = 0; v < DIMENSION; v++) {
        fill_again: do {
            d->variables[v] = field_reduce(rand() * rand() * rand());
        } while (!d->variables[v] || d->variables[v] == 1);
        for (int j = 0; j < v; j++)
            if (d->variables[j] == d->variables[v])
                goto fill_again;
    }


    for (v = 0; v < DIMENSION; v++)
        d->base[v] = 1;
    d->all_ones_value = eval_target(d);

    /* determine groupies */
    for (v = 0; v < DIMENSION; v++) {
        entry the_value;
        d->base[v] = d->variables[v];
        the_value = eval_target(d);
        d->base[v] = 1;
        d->groupie[v] = v;
        for (int w = v+1; w < DIMENSION; w++) {
            d->base[w] = d->variables[v];
            if (eval_target(d) == the_value)
                d->groupie[v] = w;
            d->base[w] = 1;
        }
    }


    return 1;
}

int data_fill(data *d, bitstring *b) {
    int i, j, // matrix entries
           v, // indexes variables
           n, // matrix size
           c; // indexes subsets themselves
    int support_alloc;
    coordinate_t *new_support;
    entry value_source;
    //entry difference;

    n = d->candidate_size;

    matrix_from_f2(d->source, b, n);
    value_source = eval_source(d);

    if (field_reduce(value_source + d->all_ones_value) == 0) {
        for (i = 0; i < n; i++) {
            entry t = d->source[0][i];
            d->source[0][i] = d->source[1][i];
            d->source[1][i] = t;
        }
    } else if (field_reduce(value_source - d->all_ones_value)) {
        return -1;
    }

#if 0
    for (v = 0; v < DIMENSION; v++)
        d->base[v] = 1;
#endif

    d->maxplaced = 0;

    for (support_alloc = i = 0; i < n; i++)
        support_alloc += bs_countbits(b[i]);

    if (support_alloc < DIMENSION + PLACEONES(n))
        return -2;

    d->support_size = support_alloc;

    if (d->support_alloc < support_alloc) {
        if ( (new_support = realloc(d->support, support_alloc * sizeof(*new_support) )) ) {
            d->support = new_support;
            d->support_alloc = support_alloc;
        } else {
            DEBUG_PRINTF("! failed to (re)allocate support array.\n");
            return 0;
        }
    }

    for (c = i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (d->source[i][j]) {
                d->support[c].row = i;
                d->support[c].col = j;
                c++;
            }
        }
    }

    d->support_freeslots = d->support_size - PLACEONES(n);
    ASSERT(d->support_freeslots >= 0);

    for (v = 0; v < DIMENSION; v++)  {
        candidate_t *new_grouping;
        int s;
        int memory = (d->support_freeslots+1) * sizeof(*GROUPING(d,v).size);

        for (s = 0; s < GROUPING(d,v).cached; s++)
            free(GROUPING(d,v).size[s].entry);

        if (d->support_freeslots >= GROUPING(d,v).cached) {
            if (( new_grouping =  realloc( GROUPING(d,v).size, memory) )) {
                GROUPING(d,v).size = new_grouping;
            } else {
                DEBUG_PRINTF("! failed to (re)allocate groupings array (%d).\n", d->support_freeslots);
                return 0;
            }
        }

        memset(GROUPING(d,v).size, 0, memory);
        GROUPING(d,v).cached = 0;
    }

    return 1;
}




void variable_set(data *d, bitstring chosen, int v) {
    for (int bit = 0; bit < d->support_size; bit++)
        if (bs_test(chosen, bit))
            d->source[d->support[bit].row][d->support[bit].col] = d->variables[v];
}

void variable_clr(data *d, bitstring chosen) {
    for (int bit = 0; bit < d->support_size; bit++)
        if (bs_test(chosen, bit))
            d->source[d->support[bit].row][d->support[bit].col] = 1;
}


int player_playall(data *d, bitstring blocked, int freeslots, int variables_set, int minsize) {

    bitstring
        variables_to_skip,       // variables that should be skipped
        variables_already_set=0; // variables that have been set
    entry testbase[DIMENSION];

    if (*(d->abort)) return 1;

    if (variables_set == DIMENSION)
        return the_callback(d);

    memcpy(testbase, d->base, sizeof(testbase));

    // compute the variables which have already been set and store them in a
    // bitstring.
    for (int v = 0; v < DIMENSION; v++)
        if (testbase[v]!=1) variables_already_set |= (1<<v);

    for (int s = minsize; (DIMENSION - variables_set) * s <= freeslots && STEST(s);  s++) {
        variables_to_skip = variables_already_set;
        for (int v = 0; v < DIMENSION; v++) {
            entry value_target;
            if ( (variables_to_skip>>v)&1 )
                continue;

            // Choose the standard variable index for the groupings of this
            // variable. Several variables may have the same groupings, and
            // the groupie of v is the one with the largest index among all of
            // them.
            int stdv = d->groupie[v];

            // Compute the groupings up to the size (number of slots set to
            // variable) that we currently use.
            while (s > GROUPING(d,stdv).cached)
                lazy_groupings(d, stdv);

           d->base[v] = d->variables[v]; // set variable in target function
           value_target = eval_target(d);  // evaluate target function

           // Compute the variables w with a higher index that would evaluate
           // to the same value if placed in position v. It is needless to
           // check these variables in the future as it would amount to the
           // exact same computation as for v.
           for (int w = v+1; w < DIMENSION; w++) {
               if ((variables_to_skip>>w)&1) continue;
               testbase[w]=d->variables[v];
               if ((*d->target)(testbase)==value_target)
                   variables_to_skip |= (1<<w);
               testbase[w]=1;
           }

           // For all groupings
           for (int c = 0; c < GROUPING(d,stdv).size[s].count; c++) {
                if (GROUPING(d,stdv).size[s].entry[c] & blocked)
                    continue;
                variable_set(d, GROUPING(d,stdv).size[s].entry[c], v);
                if (!variables_set || value_target == eval_source(d)) {
                    if (variables_set > d->maxplaced)
                        d->maxplaced = variables_set;
                    if (player_playall(
                            d, /* the data structure */
                            blocked | GROUPING(d,stdv).size[s].entry[c],
                            freeslots - s,
                            variables_set + 1,
                            (v==DIMENSION-1)?(s+1):s
                    )) {
                        return 1;
                    }
                }
                variable_clr(d, GROUPING(d,stdv).size[s].entry[c]);
            }

            d->base[v] = 1;

        }
    }
    return 0;
}

int play(data *d) {
    return player_playall(d, 0, d->support_freeslots, 0, 1);
}


int main(int argc, char **argv) {
    unsigned char input[60];
    FILE* fp = (argc > 1) ? fopen(argv[1], "r") : stdin;
    int abort = 0;
    if (!fp) return 1;

    srand(31337);
    ASSERT( sizeof(entry) == 8*sizeof(unsigned char));
    PROGRESS_INIT;

#ifdef _OPENMP
#pragma omp parallel
#endif
{
    bitstring b[MAX_DET_SIZE];
    int n;
    data d = { 0 };

    DEBUG_PRINTF("* thread number %d is launching.\n",omp_get_thread_num());

bdctool_read_loop:
    if (fscanf(fp, "%s\n", input) != 1) {
#ifdef _OPENMP
#pragma omp atomic
#endif
        abort++;
    }

    if (!abort)
    {
        if (f2_read6_raw(input, b, &n, MAX_DET_SIZE)) {
            PROGRESS_STEP;
            if (d.candidate_size != n) {
                DEBUG_PRINTF("* resizing buffer matrix from %d to %d vertices\n", d.candidate_size, n);
                data_free(&d);
                if (!data_alloc(&d, __target, n))
#ifdef _OPENMP
#pragma omp atomic
#endif
                    abort++;
                else d.abort = &abort;
            }

            if (!abort && data_fill(&d, b)>0) {
                PROGRESS_ECHO(input);
                if (play(&d))
#ifdef _OPENMP
#pragma omp atomic
#endif
                    abort++;
            }
        } else {
            DEBUG_PRINTF("! could not decode graph [%s].\n", input);
        }
    }
    if (!abort)
        goto bdctool_read_loop;
    else
        data_free(&d);
}

    if (fp && argc > 1) fclose(fp);
    return 0;
}

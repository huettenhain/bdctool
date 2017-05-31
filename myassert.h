#include <assert.h>

#if !defined(DEBUG) && defined(_DEBUG)
#define DEBUG
#endif

#ifdef DEBUG
#define ASSERT assert
#else
#define ASSERT(_x)
#endif

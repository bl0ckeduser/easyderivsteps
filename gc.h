#ifndef GC_H
#define GC_H

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
	int alloc;
	int memb;
	void **pile;
} gcarray_t;

extern gcarray_t* new_gca(void);
extern void *cgc_malloc(size_t size);
extern void gca_cleanup(gcarray_t *gca);
extern void cgc_set(gcarray_t *gca);

#endif

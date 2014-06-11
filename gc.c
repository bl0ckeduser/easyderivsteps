/*
 * rough "garbage collector" helping system
 * (memory allocations are made into groups
 * which are freed by the main code when it
 * is clear that said memory is done with)
 */

#include "gc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

extern void fail(char* mesg);

#define USE_GC

static gcarray_t* curr = NULL;

void add_to_pile(gcarray_t *gca, void *ptr)
{
#ifdef USE_GC
	if (!gca)
		return;
	if (++(gca->memb) > gca->alloc) {
		gca->alloc += 32;
		if (!(gca->pile = realloc(gca->pile, gca->alloc * sizeof(void *))))
			fail("out of memory");
	}
	gca->pile[gca->memb - 1] = ptr;
#endif
}

void *cgc_malloc(size_t size)
{
	void *ptr = malloc(size);
#ifdef USE_GC
	add_to_pile(curr, ptr);
#endif
	return ptr;
}

void cgc_set(gcarray_t *gca)
{
	curr = gca;
}

gcarray_t* new_gca(void)
{
	gcarray_t *g = malloc(sizeof(gcarray_t));
	if (!g)
		fail("out of memory");
	g->pile = malloc(8 * sizeof(void *));
	if (!(g->pile))
		fail("out of memory");
	g->memb = 0;
	g->alloc = 8;
	return g;
}

void gca_cleanup(gcarray_t *gca)
{
	int i;
#ifdef USE_GC
	for (i = 0; i < gca->memb; ++i)
		free(gca->pile[i]);

	free(gca->pile);
	free(gca);
#endif
}



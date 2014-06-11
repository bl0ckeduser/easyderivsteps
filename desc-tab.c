#include "desc-tab.h"
#include "gc.h"

/*
 * This is (essentially) a dynamic array of strings.
 * It is used to stored descriptive comments,
 * i.e. the stuff that gets displayed in blue.
 * It is indexed by rule-number and is always
 * passed around with the associated rule-table.
 */

/*
	typedef struct {
		int a;		<- alloc
		int c;		<- count
		char **d;	<- entries
	} desc_tab;

	extern desc_tab* desc_tab_new(void);
	extern desc_tab* desc_tab_add(char *d, int id);

*/

#include <stdio.h>

extern void fail(char* mesg);

desc_tab* desc_tab_new(void)
{
	desc_tab *dt = cgc_malloc(sizeof(desc_tab));
	int i;
	if (!dt)
		fail("Out of memory");
	dt->d = malloc(10 * sizeof(char *));
	if (!(dt->d))
		fail("Out of memory");
	for (i = 0; i < 10; ++i) {
		dt->d[i] = NULL;
	}
	dt->a = 10;
	dt->c = 0;

	return dt;
}

void desc_tab_add(desc_tab *dt, char *d, int id)
{
	int i;
	int old;

	if (id >= dt->a) {
		old = dt->a;
		dt->a = id + 8;
		dt->d = realloc(dt->d, dt->a * sizeof(char *));
		for (i = old; i < dt->a; ++i) {
			dt->d[i] = NULL;
		}

		if (!(dt->d))
			fail("Out of memory");
	}
	if (id >= dt->c)
		dt->c = id + 1;
	dt->d[id] = malloc(strlen(d) + 1);
	if (!(dt->d[id]))
		fail("Out of memory");
	strcpy(dt->d[id], d);

	/* deal with the fucking newline */
	for (i = 0; dt->d[id][i]; ++i)
		if (dt->d[id][i] == '\n') {
			dt->d[id][i] = 0;
			break;
		}

	/* printf("dt add %d, '%s', %p, %p\n", id, dt->d[id], dt, dt->d[id]); */
}



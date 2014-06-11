#ifndef DTAB_H
#define DTAB_H

typedef struct {
	int a;	/* alloc */
	int c;	/* count */
	char **d;	/* data */
} desc_tab;

extern desc_tab* desc_tab_new(void);
extern void desc_tab_add(desc_tab *dt, char *d, int id);

#endif

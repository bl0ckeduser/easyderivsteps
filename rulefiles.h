#ifndef RULEFILE_H
#define RULEFILE_H

#include "desc-tab.h"

extern void rule(char *r, exp_tree_t **rules, int* rc);
extern int readrules(exp_tree_t** rules, desc_tab *dt, char *dir);

#endif

#ifndef APPLYRULES_H
#define APPLYRULES_H

#include "desc-tab.h"

extern int apply_rules_and_optimize(exp_tree_t** rules, desc_tab* dt, int rc, exp_tree_t *t);

#endif

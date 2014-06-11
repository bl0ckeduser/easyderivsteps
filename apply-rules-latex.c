#include "gc.h"

#include "tree.h"
#include "dict.h"
#include "match.h"
#include "optimize.h"

#include <stdio.h>
#include "desc-tab.h"
extern int count;
extern int first_only;
extern FILE *texout;
extern exp_tree_t *root;

/* We recursively iterate through all 
 * rules on all the children of the tree
 * until the expression is irreducible, 
 * i.e. stops changing.
 * 
 * Also, we (try to) apply the optimization
 * routine.
 */

extern int hack;	/* use: see below */

extern void printout_tree_latex(exp_tree_t et, desc_tab *dt);

int apply_rules_and_optimize(exp_tree_t** rules, desc_tab* dt, int rc, exp_tree_t *tree)
{
	int success = 0;
	while (1) {
		if (matchloop(rules, dt, rc, tree)) {
			/*
			 * Reduction algorithm
			 * suceeded, print reduced tree
			 */
			success = 1;
			/* Bail out early if only preview desired */
			if (first_only && count)
				return success;
		}

		/*
		 * If optimization succeeds in
		 * modifying the tree, try
		 * reducing the new optimized
		 * tree.
		 */
		if (optimize(tree)) {
			success = 1;
			
			/* Bail out early if only preview desired */
			if (first_only && count)
				return success;
			printout_tree_latex(*root, dt);
			continue;
		}
		break;
	}

	if (success) {
		/* 
		 * `hack' turns off optimizations that do
		 * not have any effect on the graphical
		 * rendered appearance of the expression
		 */
		printout_tree_latex(*root, dt);
		hack = 1;
		while (optimize(tree))
			;
		hack = 0;
		printout_tree_latex(*root, dt);
	}

	return success;
}

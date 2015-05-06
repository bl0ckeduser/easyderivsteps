#include "gc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "tokens.h"
#include "tree.h"
#include "parser.h"
#include "dict.h"
#include "subst.h"
#include "optimize.h"
#include "match.h"
#include "infix-printer.h"
#include "apply-rules.h"
#include "rulefiles.h"
#include "gc.h"

extern void printout_tree_latex(exp_tree_t et, desc_tab *dt);

FILE *texout;
exp_tree_t *root;

char* buf;
token_t* tokens;
exp_tree_t tree;
int mc;
int i;
exp_tree_t* rules[128];
exp_tree_t* pres_rules[128];
int rc;
int prc;
char lin[1024];
int success;
gcarray_t *setup, *iter;

extern void fail(char*);
extern int unwind_expos(exp_tree_t *et);

char *full;

desc_tab *dt1;
desc_tab *dt2;

/*
 * This gets the lexical analyser ready to run,
 * and it also reads to memory the rules and
 * rule-comments.
 */
void do_setup()
{
	cgc_set(setup = new_gca());

	dt1 = desc_tab_new();
	dt2 = desc_tab_new();

	/*
	 * Setup lexer (wannabe regex code)
	 */
	setup_tokenizer();

	/*
	 * Read rules stored in "rules"
	 * directory. Boring file processing
	 * stuff that calls the parser and
 	 * stores parsed trees into an array.
	 * Code is data. Data is code.
	 */
	rc = readrules(rules, dt1, "rules");

	/* There are also rules for presentation
	 * of the final result, for example
	 * the notation e^x is preferred to
	 * the notation exp(x), etc.
	 */
	prc = readrules(pres_rules, dt2, "pres-rules");
}

/*
 * Check if a tree has a differentiation
 * operator-node in it. This is applied
 * on the final-step result to
 * check if differentiation failed.
 */
int check_incomplete(exp_tree_t *et)
{
	int i = 0;

	if (et->head_type == FUNC
		&& et->tok->len == 1
		&& et->tok->start[0] == 'D')
		return 1;

	for (i = 0; i < et->child_count; ++i) 
		if (check_incomplete(et->child[i]))
			return 1;

	return 0;
}

int parencheck(char *lin)
{
	int depth = 0;
	for (; *lin; ++lin) {
		if (*lin == '(')
			++depth;	
		else if (*lin == ')')
			if (--depth < 0)
				return 0;
	}
	return depth == 0;
}

char *doit(int run_main_program, char *lin)
{
	full = malloc(1024 * 2);
	*full = 0;
	int opt = 0;

	do {
		/* Create a GC "zone" for the work done herein */
		cgc_set(iter = new_gca());

		if (!parencheck(lin)) {
			fail_alert("Imbalanced parentheses in input");
		}

		/* Lex the line */
		tokens = tokenize(lin);

		/* Parse the tokens into an expression-tree */
		tree = *((parse(tokens)).child[0]);
		root = &tree;

		printout_tree(tree);
		printf("\n");

		if (apply_rules_and_optimize(rules, dt1, rc, &tree)) {
			/* Some final clean-up ... */
			while(unwind_expos(&tree))
				;
			(void)apply_rules_and_optimize(pres_rules,
				dt2, prc, &tree);

			while(optimize(&tree))
				/*
				 * 	  2014-01-27
				 *
				 * kludgy fix for an infinite loop
				 * in the "preview" button
				 */
				if (!run_main_program && opt++ > 5)
					break;

			printout_tree_latex(tree, NULL);
		}

		/* 
		 * If there is still a d/d[...] left in the last
		 * line of output, something went wrong and it is
		 * a good idea to put up a special help-message
		 * in the JS GUI.
		 */
		if (run_main_program)
			if (check_incomplete(&tree))
				emscripten_run_script("diff_incomplete()");

		/* Garbage-collection for the work in this "zone" */
		gca_cleanup(iter);
	} while(0);

	return full;
}

void do_cleanup()
{
	gca_cleanup(setup);
}

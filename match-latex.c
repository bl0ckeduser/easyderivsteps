#include "gc.h"
#include "desc-tab.h"

#include <ctype.h>

/*
 * Tree pattern-matching algorithm,
 * based on the one described in the SICP video
 * about pattern-matching ("4A: Pattern Matching and 
 * Rule-based Substitution").
 *
 * The nasty extensions are not from SICP.
 */

#include "tree.h"
extern void write_tree_latex(exp_tree_t et, char *buff);
extern int hack;
extern void printout_tree_latex(exp_tree_t et, desc_tab *dt);

extern gcarray_t *setup, *iter;

#include "tree.h"
#include "tokenizer.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>
#include "optimize.h"
#include <stdio.h>
#include "dict.h"
#include "subst.h"
#include "infix-printer.h"

extern int count;
extern int first_only;

int last_succ_rule = -1;

extern void fail(char* mesg);
extern int sametree(exp_tree_t *a, exp_tree_t *b);

char buf_a[128], buf_b[128], buf_c[128];

extern FILE *texout;
extern exp_tree_t *root;

char *step_desc;

/* Is "tree" not identical to "key"
 * or a father (recursively) of such a tree ?
 *
 * This is used for special pattern-matching
 * syntaxes like e:x and e|x, which only match
 * an expression when it contains (or not)
 * a chosen sub-expression.
 */
int indep(exp_tree_t *tree, exp_tree_t *key)
{
	int i;

	if(sametree(tree, key))
		return 0;

	for (i = 0; i < tree->child_count; ++i) {
		if (sametree(tree->child[i], key))
			return 0;
		if (tree->child[i]->child_count)
			if(!indep(tree->child[i], key))
				return 0;
	}

	return 1;
}

exp_tree_t* find_subtree(exp_tree_t *tree, exp_tree_t *key)
{
	int i;
	exp_tree_t *catch;

	if(sametree(tree, key))
		return tree;

	for (i = 0; i < tree->child_count; ++i) {
		if (sametree(tree->child[i], key))
			return tree->child[i];
		if (tree->child[i]->child_count)
			if((catch = find_subtree(tree->child[i], key)) != NULL)
				return catch;
	}

	return NULL;
}

int same_tokens(exp_tree_t *a, exp_tree_t *b)
{
	strncpy(buf_a, b->tok->start, b->tok->len);
	buf_a[b->tok->len] = 0;

	strncpy(buf_b, a->tok->start, a->tok->len);
	buf_b[a->tok->len] = 0;

	return !strcmp(buf_a, buf_b);
}

/* 
 * This routine tries to match the tree 'a' against
 * the pattern 'b'.
 * 
 * If the routine returns true (1), the dictionary 'd'
 * has been populated with the symbol-mappings for the
 * match.
 */ 
int treematch(exp_tree_t *a, exp_tree_t *b, dict_t* d)
{
	int i;
	int id;
	int check;

	/* 
	 * VARIABLE: general (any tree) match, 
	 * MATCH_VAR: variable match,
	 * MATCH_NUM: number match
	 * MATCH_NAN: match if not number
	 * SYMBOL: match exact variable name
	 * MATCH_IND_SYM: match if not exact variable name
	 */
	if (b->head_type == VARIABLE
	|| (b->head_type == MATCH_NUM && a->head_type == NUMBER)
	|| (b->head_type == MATCH_VAR && a->head_type == VARIABLE)
	|| (b->head_type == MATCH_NAN && a->head_type != NUMBER)
	|| (b->head_type == SYMBOL && a->head_type == VARIABLE
		&& same_tokens(a, b))) {
		strncpy(buf_a, b->tok->start, b->tok->len);
		buf_a[b->tok->len] = 0;
		if ((id = dict_search(d, buf_a)) != -1) {
			if (!sametree(d->match[id], a))
				return 0;
			return 1;
		} else {
			strcpy(d->symbol[d->count], buf_a);
			d->match[d->count] = a;
			if (++d->count > d->alloc)
				expand_dict(d, d->count + 10);
			return 1;
		}
	}

	/* (MATCH_XXX (VARIABLE:e) (VARIABLE:x)) */
	/* MATCH_IND: match "E" if it contains no occurence of the symbol
	 * stored for "X" */
	/* MATCH_CONT: match "E" if it contains an occurence, or is directly
	 * the symbol stored for "X" */
	/* MATCH_CONTX: match "E" if it contains an occurence, and isn't
	 * directly the symbol stored for "X" */


/*
	(VARIABLE:bar)
	(MATCH_IND_SYM (VARIABLE:a) (VARIABLE:e))
	Match anything that isn't the symbol
*/
	if (b->head_type == MATCH_IND_SYM) {
		if (a->head_type != VARIABLE) {
			strncpy(buf_a, b->child[0]->tok->start, b->child[0]->tok->len);
			buf_a[b->child[0]->tok->len] = 0;
			strcpy(d->symbol[d->count], buf_a);
			d->match[d->count] = a;
			if (++d->count > d->alloc)
				expand_dict(d, d->count + 10);
			return 1;
		} else {
			if (same_tokens(a, b->child[1]))
				return 0;
			else {
				strncpy(buf_a, b->child[0]->tok->start, b->child[0]->tok->len);
				buf_a[b->child[0]->tok->len] = 0;
				strcpy(d->symbol[d->count], buf_a);
				d->match[d->count] = a;
				if (++d->count > d->alloc)
					expand_dict(d, d->count + 10);
				return 1;
			}
		}
	}

	if (b->head_type == MATCH_IND
		|| b->head_type == MATCH_CONT
		|| b->head_type == MATCH_CONTX) {
		/* find symbol stored for "X" */
		strncpy(buf_a, b->child[1]->tok->start, b->child[1]->tok->len);
		buf_a[b->child[1]->tok->len] = 0;
		if ((id = dict_search(d, buf_a)) != -1) {
			/* is this expression indepedent of the symbol ? */
			/* (or non-indepedent, etc) */
			if ((b->head_type == MATCH_IND
			    && indep(a, d->match[id]))
			|| (b->head_type == MATCH_CONT
			    && !indep(a, d->match[id]))
			|| (b->head_type == MATCH_CONTX
			    && !indep(a, d->match[id])
			    && !sametree(a, d->match[id]))) {
				/* yes, do standard symbolmatching */
				strncpy(buf_a, b->child[0]->tok->start,
					b->child[0]->tok->len);
				buf_a[b->child[0]->tok->len] = 0;
				if ((id = dict_search(d, buf_a)) != -1) {
					if (!sametree(d->match[id], a))
						return 0;
					return 1;
				} else {
					strcpy(d->symbol[d->count], buf_a);
					d->match[d->count] = a;
					if (++d->count > d->alloc)
						expand_dict(d, d->count + 10);
					return 1;
				}
			}
		} else {
			/* Problematic situation for which there
			 * may be a remedy... In other words,
			 * probably-fixable bug.
			 *
			 * See what "SYNTAX" file says about bug with
			 * e|x and e:x syntax.
			 */
			printf("[forward substitution issue]\n");
			return 0;
		}
	}

	/* (MATCH_FUNC:foo (ADD (VARIABLE:x) (NUMBER:1))) */
	/* MATCH_FUNC: match a function and then do standard
	 * matching on its children */
	if (b->head_type == MATCH_FUNC && a->head_type == FUNC) {
		/* get meta-symbol */
		strncpy(buf_a, b->tok->start, b->tok->len);
		buf_a[b->tok->len] = 0;

		/* find function name in pattern */
		strncpy(buf_b, a->tok->start, a->tok->len);
		buf_b[a->tok->len] = 0;

		/* already in dictionary ? */
		if ((id = dict_search(d, buf_a)) != -1) {
			/* same function name ? */
			strncpy(buf_c, d->match[id]->tok->start,
				d->match[id]->tok->len);
			buf_c[d->match[id]->tok->len] = 0;
			if (strcmp(buf_c, buf_b))
				return 0;	/* inconsistent */
		} else {
			/* not in dictionary, add it */
			strcpy(d->symbol[d->count], buf_a);
			d->match[d->count] = a;
			if (++d->count > d->alloc)
				expand_dict(d, d->count + 10);
		}

		/* don't forget to match the children !!! */
		if (a->child_count != b->child_count)
			return 0;
		for (i = 0; i < a->child_count; ++i) {
			if (!treematch(a->child[i], b->child[i], d))
				return 0;
		}
		return 1;
	}

	/* trivial non-matching cases */
	if (a->head_type != b->head_type)
		return 0;

	if (a->child_count != b->child_count)
		return 0;

	/* compare token-strings */
	if (a->tok && b->tok) {
		strncpy(buf_a, a->tok->start, a->tok->len);
		buf_a[a->tok->len] = 0;
		strncpy(buf_b, b->tok->start, b->tok->len);
		buf_b[b->tok->len] = 0;
		if (strcmp(buf_a, buf_b))
			return 0;
	}

	/* recurse onto children */
	for (i = 0; i < a->child_count; ++i) {
		if (!treematch(a->child[i], b->child[i], d))
			return 0;
	}

	return 1;
}

/*
 * Recursively iterate through all rules on all the
 * children of the tree until the
 * expression is irreducible, i.e. stops
 * changing.
 */
int matchloop(exp_tree_t** rules, desc_tab* dt, int rc, exp_tree_t* tree)
{
	dict_t *dict;
	exp_tree_t *skel;
	int i;
	int j, k, s, t;
	int mc;
	int m = 0;
	int key;
	int l = 0;
	char* buff /* [1024 * 2] */;
	char* buff2 /* [256] */;
	exp_tree_t *copy;
	int ptr_issue = 0;

	if (tree == root) {
		ptr_issue = 1;
	}

restart:
	do {
		mc = 0;
		
		/* Make a dictionary */
		dict = new_dict();

		/* Go through the rules */
		for (i = 0; i < rc; ++i) {

			/* Empty dictionary if necessary */
			if (dict->count)
				dict->count = 0;

			if (special(tree))
				m = 1;
			else if (treematch(tree, rules[i]->child[0], dict)) {
				/* It matches. Do the correspoding substitution
				 * and make a little printout of the form:
				 * [rule number] original tree -> substituted
				 *				  tree
				 */
				++mc;
				m = 1;

				last_succ_rule = i;

#ifdef JSDEBUG
				buff2 = malloc(1024);
				sprintf(buff2, "alert('rule %d')", i);
				emscripten_run_script(buff2);
				free(buff2);
#endif

				printf("rule %d\n", i);

				/*
				 * The following `if' is one of the several
				 * little hacks that got added to symdiff for the
				 * "step-by-step" GUI program.
				 * 
				 * It writes descriptive comments strings,
				 * performing subsitutions:
				 * 
				 * 		*x		=> latex rendering for x,
				 *				   enclosed in $ $ markers
				 *
				 * 		?x		=> latex rendering for x,
				 *				   enclosed in latex [ ] brackets
				 *				   if x is not a variable
				 *
				 * 		#x		=> latex rendering for x,
				 * 				   enclosed in latex (  ) parens
				 * 				   if x is not a variable
				 */
				if (i < dt->c && dt->d[i]) {
					buff2 = malloc(1024);
					*step_desc = 0;
					for (j = 1; dt->d[i][j]; ++j) {
						/* If the next character is not a substitution
						 * marker, just copy this it to the final string
						 */
						if (dt->d[i][j] != '*' && dt->d[i][j] != '&' && dt->d[i][j] != '?'
							&& dt->d[i][j] != '#') {
							sprintf(step_desc, "%s%c", step_desc, dt->d[i][j]);
						} else {
							/* Hit a substitution marker. */
							t = dt->d[i][j];
							++j;	/* eat the marker ? */
							buff2[0] = 0;
							for (s = 0; ; ++s) {
								if (dt->d[i][j] && isalpha(dt->d[i][j])) {
									buff2[s] = dt->d[i][j];
									++j;
								} else
									break;
							}
							buff2[s] = 0;
							if (dict->count && (key = dict_search(dict, buff2)) != -1) {
								write_tree_latex(*(dict->match[key]), buff2);
								if (t == '*')
									sprintf(step_desc, "%s $ ", step_desc);

								if (t == '?' && dict->match[key]->head_type != VARIABLE)
									sprintf(step_desc, "%s \\\\left[", step_desc);

								if (t == '#' && dict->match[key]->head_type != VARIABLE)
									sprintf(step_desc, "%s \\\\left(", step_desc);

								strcat(step_desc, buff2);

								if (t == '*')
									sprintf(step_desc, "%s $ ", step_desc);

								if (t == '?' && dict->match[key]->head_type != VARIABLE)
									sprintf(step_desc, "%s \\\\right] ", step_desc);

								if (t == '#' && dict->match[key]->head_type != VARIABLE)
									sprintf(step_desc, "%s \\\\right) ", step_desc);

								puts(buff2);
							}
							/* argh wtf */
							if (dt->d[i][j] != '*' && dt->d[i][j] != '&' && dt->d[i][j] != '?'
								&& dt->d[i][j] != '#')
								sprintf(step_desc, "%s%c", step_desc, dt->d[i][j]);
						}
					}
					puts(step_desc);
					free(buff2);
				}


				if (dict->count) {
					/* Substitution with symbols */
					skel = copy_tree(rules[i]->child[1]);
					subst(skel, dict, &skel);
					*tree = *skel;
				} else {
					/* Plain direct substitution */
					*tree = *copy_tree(rules[i]->child[1]);
				}

				printout_tree_latex(*root, dt);

				/* 
				 * Exits early when only a preview
				 * is desired.
				 */
				if (first_only && count)
					return 1;

				break;
			}
		}
	/* If any of the rules matched, try them all again. */
	} while (mc);

	/* Do matching loop on all children. If it succeeds,
	 * do the loop again on this tree. */
	for (i = 0; i < tree->child_count; ++i)
		if (matchloop(rules, dt, rc, tree->child[i])) {
			m = 1;
			/* goto restart; */
		} 
	return m;
}


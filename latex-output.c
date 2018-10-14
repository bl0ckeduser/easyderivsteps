#include "gc.h"
#include "desc-tab.h"

/*
 * Print out an expression tree in LATEX.
 *
 * It does some prettifications:
 *	5 * x ^ 4 => 5x^4
 * 	A + -B    => A - B
 *
 * And it also makes sure to never print the
 * same line twice in a row.
 */

/* TODO:
 *		 	-1(
 *			+1(
 */

int color_off = 0;

extern char *step_desc;

#include "tree.h"
#include "tokenizer.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern int first_only;

extern char *display_div;
extern char *full;
char name[128];
char tmp[256];
int count = 0;

extern int last_succ_rule;

/*
 * Helper routine: check precedence of an
 * operator. (Used by the unnecessary-
 * parenthesis-omission code below).
 */
int ltxo_op_prec(int head_type)
{
	switch (head_type) {
		case EQL:	return 1;
		case EXP:	return 2;
		case DIV:	return 3;
		case MULT:	return 3;
		case SUB:	return 4;
		case ADD:	return 4;
		default:	return 0;
	}
}

/* Helper: tree type -> operator symbol */
char* ltxo_opsym(int head_type)
{
	switch (head_type) {
		case MULT:	return /* " \\cdot " */ " \\\\cdot ";
		case DIV:	return " / ";
		case SUB:	return " - ";
		case ADD:	return " + ";
		case EQL:	return " = ";
		case EXP:	return "^";
	}
	return " ";
}

/* turns +- to - */
void cleanup(char *orig, char* new)
{
	/* printf("cleanup: %s\n", orig); */
	int i = 0;
	int j = 0;
	int ops = 0;
	int last;
/*	strcpy(new, orig);
	return;
*/

	for (i = 0; orig[i]; i++) {
		if (ops == '+' && orig[i] == '-') {
			j = last - 1;
		}
		if (orig[i] == '+' || orig[i] == '-') {
			ops = orig[i];
			last = i;
		}
		if (orig[i] != ' ' && orig[i] != '+' && orig[i] != '-') {
			ops = 0;
		}

		new[j++] = orig[i];
	}
	new[j] = 0;
}

/* Write latex-rendering of a tree to a string */
void write_tree_latex(exp_tree_t et, char *buff)
{
	extern void printout_tree_latex_derp(char *buf, exp_tree_t et, int pp);
	char tmp[1024 * 1024];
	tmp[0] = 0;
	buff[0] = 0;
	color_off = 1;
	printout_tree_latex_derp(tmp, et, 0);
	color_off = 0;
	cleanup(tmp, buff);
}

/*
 * 2014-04-23
 * 
 * This routine removes certain types of junk nodes
 * immediately before any rendering
 * calls are done. The optimizer handles
 * many such problems, but the GUI is programmed to
 * not run more than one optimizer iteration
 * at once. Result: garbage output like "1 \cdot "
 * in certain cases.
 */
void prune(exp_tree_t *p)
{
	int i;
	if (p->head_type == DIV
		&& p->child_count == 1)
		memcpy(p, p->child[0], sizeof(*p));

	for (i = 0; i < p->child_count; ++i)
		prune(p->child[i]);
}

/* Write latex-rendering to the JS GUI.
 * The destination DIV is specified by the
 * global variable display_div, which is owned
 * by jsmain.c.
 * 
 * All this function really does is some boring 
 * book-keeping and a bunch of terrible ugly hacks
 * to prevent as best as possible repetition of lines.
 *
 * The actual latex printing is done by 
 * printout_tree_latex_derp,
 * which is a relatively clean recursive routine.
 */
void printout_tree_latex(exp_tree_t et, desc_tab *dt)
{
	extern void printout_tree_latex_derp(char *buf, exp_tree_t et, int pp);
	char buf[1024 * 1024];
	char buf2[1024 * 1024];
	char buf3[1024 * 1024];

	prune(&et);

/*
	static char last[1024 * 1024] = "";	
	static char last_color[1024 * 1024] = "";	
*/
	static char* last = NULL;
	static char* last_color = NULL;

	if (!last) {
		last = malloc(1024 * 1024 * 2); 
		*last = 0;
	}
	if (!last_color) {
		last_color = malloc(1024 * 1024 * 2);
		*last_color = 0;
	}

	/* clear last on new first iteration */
	if (count == 0)
		*last = 0;

	/* 
	 * Bail out if only a "preview"
 	 * (i.e. the first line)
	 * is desired.
	 */
	if (first_only && count)
		return;

	buf[0] = 0;
	printout_tree_latex_derp(buf, et, 0);
	cleanup(buf, buf2);

	color_off = 1;
	buf[0] = 0;
	printout_tree_latex_derp(buf, et, 0);
	cleanup(buf, buf3);
	color_off = 0;

	if (strlen(buf2) >= 2) {
		if (count && strcmp(buf2, last_color) != 0)
			++count;

		/* don't repeat lines !!!!! */
		if (strcmp(buf3, last) != 0 && (count == 0 || strstr(buf2, "Red"))) {
			full[0] = 0;
#ifndef DESC_GEN
			strcat(full, "document.getElementById('");
			strcat(full, display_div);
			strcat(full, "').innerHTML += '");
#endif			
			/* descriptive string available;
			 * write it out in blue. */
			if (count && *step_desc) {
				strcat(full, "<br><br>");
				strcat(full, "<font color=\\'' + getDescriptiveTextColorForTheme() + '\\'>");
				strcat(full, step_desc);
				strcat(full, "</font>");
			}
#ifndef DESC_GEN
			if (count)
				strcat(full, "<br><br> $ ");
			else
				strcat(full, "<br> $ ");
#else
			strcat(full, "\n $ ");
#endif

			if (count++)
				strcat(full, " = ");
#ifndef DESC_GEN
			strcat(full, buf2);
#else
			strcat(full, buf3);
#endif
#ifndef DESC_GEN
			strcat(full, " $'");
#else
			strcat(full, " $");
#endif
			emscripten_run_script(full);
		}
	}

	puts(full);
	printout_tree(et);
	printf("\n");

	strcpy(last_color, buf2);

	color_off = 1;
	buf[0] = 0;
	printout_tree_latex_derp(buf, et, 0);
	cleanup(buf, buf2);
	strcpy(last, buf2);
	color_off = 0;

	*step_desc = 0;
}

/*
 * Write-out the latex rendering of a tree
 * Recurses onto children...
 */
void printout_tree_latex_derp(char *buf, exp_tree_t et, int pp)
{
	int i;
	int parens_a = 0;
	int parens_b = 0;

	/* 
	 * Write things that have changed in this 
	 * step-iteration in red. 
	 */
	if (et.age == count && count && !color_off)
		strcat(buf, "\\\\color{Red}{");

	/* 'foo(bar, donald, baker)' */
	if (et.head_type == FUNC) {
		strncpy(name, (et.tok)->start, (et.tok)->len);
		name[(et.tok)->len] = 0;

		if (strcmp(name, "sqrt") == 0) {

			sprintf(tmp, "\\\\sqrt{");
			strcat(buf, tmp);

			printout_tree_latex_derp(buf, *(et.child[0]), 0);

			strcat(buf, "}");

		} else if (strcmp(name, "D") == 0) {
			/* \frac{d}{d{u}}\left[{u^2}\right] */
			
			strcat(buf, "\\\\frac{d}{d{");
			if (et.child[0]->head_type != VARIABLE) {
				strcat(buf, "\\\\left[{");
			}
			printout_tree_latex_derp(buf, *(et.child[0]), 0);
			if (et.child[0]->head_type != VARIABLE) {
				strcat(buf, "}\\\\right]");
			}
			strcat(buf, "}}\\\\left[{");
			printout_tree_latex_derp(buf, *(et.child[1]), 0);
			strcat(buf, "}\\\\right]");
		} else {

			/* 
			 * Typeset recognized functions in
			 * non-italics by putting a backslash 
			 * before them
			 */
			if (!strcmp(name, "log")
			|| !strcmp(name, "ln")
			|| !strcmp(name, "sin")
			|| !strcmp(name, "cos")
			|| !strcmp(name, "tan")
			|| !strcmp(name, "csc")
			|| !strcmp(name, "cot")
			|| !strcmp(name, "sec")
			|| !strcmp(name, "arcsin")
			|| !strcmp(name, "arccos")
			|| !strcmp(name, "arctan")
			/* latex doesnt officialy recognize these */
			/* || !strcmp(name, "arccsc")
			 * || !strcmp(name, "arccot")
			 * || !strcmp(name, "arcsec") */) {
				strcat(buf, "\\\\");
			}

			sprintf(tmp, "%s", name);
			strcat(buf, tmp);

			strcat(buf, "(");

			for (i = 0; i < et.child_count; i++) {
				if (i)
					strcat(buf, ", ");
				printout_tree_latex_derp(buf, *(et.child[i]), 0);
			}

			strcat(buf, ")");
		}

		if (et.age == count && count && !color_off)
			strcat(buf, "}");

		return;
	}

	/* 'bar' */
	/* '123' */
	if (et.head_type == VARIABLE
	|| et.head_type == NUMBER) {
		strncpy(name, (et.tok)->start, (et.tok)->len);
		name[(et.tok)->len] = 0;
		sprintf(tmp, "%s", name);
		strcat(buf, tmp);
		if (et.age == count && count && !color_off)
			strcat(buf, "}");
		return;
	}

	/* '-X' */
	if (et.head_type == NEGATIVE) {
		strcat(buf, "-");
		printout_tree_latex_derp(buf, **et.child, 0);
		if (et.age == count && count && !color_off)
			strcat(buf, "}");
		return;
	}

	if(et.head_type == DIV && et.child_count == 2) {
		strcat(buf, "\\\\frac{");
		printout_tree_latex_derp(buf, *(et.child[0]), 0);
		strcat(buf, "}{");
		printout_tree_latex_derp(buf, *(et.child[1]), 0);
		strcat(buf, "}");
		if (et.age == count && count && !color_off)
			strcat(buf, "}");
		return;
	}

	/* 'A + B + C' */
	if (et.head_type == MULT
	|| et.head_type == ADD
	|| et.head_type == SUB
	|| et.head_type == EQL
 	|| (et.head_type == DIV && et.child_count > 2)
	|| et.head_type == EXP) {
		/* 
		 * Need parentheses if child has lower precedence.
		 * e.g. (1 + 2) * 3
		 */
		parens_a = pp && pp < ltxo_op_prec(et.head_type);
		if (parens_a) {
			strcat(buf, "(");
		}

		for (i = 0; i < et.child_count; i++) {
			/* 
			 * Check for the second kind of need-of-parentheses,
			 * for example in expressions like 1 / (2 / 3) 
			 */
			parens_b = ltxo_op_prec(et.head_type) == 
					ltxo_op_prec(et.child[i]->head_type)
			    && et.child[i]->child_count
				&& ltxo_op_prec(et.head_type)
				&& et.head_type != MULT
				&& et.head_type != ADD;

			/* x^{y^z} */
			if (et.head_type == EXP) {
				strcat(buf, "{");
			}

			if (et.head_type == EXP
				&& et.child[i]->head_type == DIV)
				strcat(buf, "\\\\left(");

			if (parens_b)
				strcat(buf, "(");

			if ((et.head_type == MULT
				&& et.child[i]->head_type == NUMBER
				&& tok2int(et.child[i]->tok) == -1
				&& i == 0)
||
				(et.head_type == MULT
				&& et.child[i]->head_type == NEGATIVE
				&& et.child[i]->child_count == 1
				&& et.child[i]->child[0]->head_type == NUMBER
				&& tok2int(et.child[i]->child[0]->tok) == 1
				&& i == 0)

			) {
				strcat(buf, " - ");
			} else {
				/* print child */
				printout_tree_latex_derp(buf, *(et.child[i]), ltxo_op_prec(et.head_type));
			}

			if (parens_b) {
				strcat(buf, ")");
			}

			if (et.head_type == EXP
				&& et.child[i]->head_type == DIV)
				strcat(buf, "\\\\right)");

			/* x^{y^z} */
			if (et.head_type == EXP) {
				strcat(buf, "}");
			}

			/* 
			 * In infix notation, an operator symbol comes
			 * after every child except the last
			 */
			if (i < et.child_count - 1) {
				if (0
||
				(et.head_type == MULT
				&& et.child[i]->head_type == NUMBER
				&& et.child[i+1]->head_type == EXP
				&& et.child[i+1]->child_count == 2
				&& et.child[i+1]->child[0]->head_type == VARIABLE)
||
				(et.head_type == MULT
				&& et.child[i]->head_type == NEGATIVE
				&& et.child[i]->child_count == 1
				&& et.child[i]->child[0]->head_type == NUMBER
				&& et.child[i+1]->head_type == EXP
				&& et.child[i+1]->child_count == 2
				&& et.child[i+1]->child[0]->head_type == VARIABLE)
||
				(et.head_type == MULT
				&& et.child[i]->head_type == NUMBER
				&& et.child[i+1]->head_type == VARIABLE)
||
				(et.head_type == ADD
				&& et.child[i+1]->head_type == NEGATIVE)
||
				(et.head_type == ADD
				&& et.child[i+1]->head_type == MULT
				&& et.child[i+1]->child[0]->head_type == NEGATIVE)
||
				(et.head_type == ADD
				&& et.child[i+1]->head_type == MULT
				&& et.child[i+1]->child[0]->head_type == NUMBER
				&& et.child[i+1]->child[0]->tok->start[0] == '-')

||				(et.head_type == ADD
				&& et.child[i+1]->head_type == NUMBER
				&& et.child[i+1]->tok->start[0] == '-')

||				(et.head_type == ADD
				&& et.child[i+1]->head_type == MULT
				&& et.child[i+1]->child[0]->head_type == NUMBER
				&& et.child[i+1]->child[0]->tok->start[0] == '-')

||				(et.head_type == ADD
				&& et.child[i+1]->head_type == MULT
				&& et.child[i+1]->child[0]->head_type == NEGATIVE)

||				(et.head_type == MULT
				&& et.child[i]->head_type == NUMBER
				&& tok2int(et.child[i]->tok) == -1
				&& i == 0)
||
				(et.head_type == MULT
				&& et.child[i]->head_type == NEGATIVE
				&& et.child[i]->child_count == 1
				&& et.child[i]->child[0]->head_type == NUMBER
				&& tok2int(et.child[i]->child[0]->tok) == 1
				&& i == 0)

				) {
					/* 
					 * Omissions of operators for neatness...
					 * There are several cases...
					 */
				} else {
					strcat(buf, ltxo_opsym(et.head_type));
				}
			}
		}

		/* close first type of parentheses-need */
		if (parens_a) {
			strcat(buf, ")");
		}
		if (et.age == count && count && !color_off)
			strcat(buf, "}");

		return;
	}

	/* oh boy */
	if (et.age == count && count && !color_off)
		strcat(buf, "}");
}



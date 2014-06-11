#include "gc.h"

/*
 * This is pretty boring code which reads
 * all the lines in all the files in the
 * directory "rules", parses them and
 * adds the resulting trees into the
 * rule-list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "tokens.h"
#include "tree.h"
#include "parser.h"
#include "dict.h"
#include <dirent.h>
#include <unistd.h>
#include "rulefiles.h"
#include "optimize.h"
#include "desc-tab.h"

extern void fail(char*);

int readrules(exp_tree_t** rules, desc_tab *dt, char *dir)
{
	DIR *dirp;
	struct dirent *dp;
	FILE* f;
	char lin[1024];
	int rc = 0;

	/* printf("Reading rules from '%s'...", dir); */
	fflush(stdout);
	if ((dirp = opendir(dir))) {
		if (chdir(dir) == -1)
			fail("directory change failed");
		for (dp = readdir(dirp); dp; dp = readdir(dirp)) {
			printf("rulefile %s/%s\n", dir, dp->d_name);
			if (*(dp->d_name) != '.') {
				if ((f = fopen(dp->d_name, "r"))) {
					while (1) {
						if (!fgets(lin, 1024, f))
							break;
						/* descriptive step-comments */
						if (*lin == ';' && lin[1]) {
							desc_tab_add(dt, lin + 1, rc - 1);
						} else 
						/* skips comments and blanks ... */
						if (!lin[1] || !*lin || *lin == '%')
							continue;
						else {
							lin[strlen(lin) - 1] = 0;
							printf("rule %d: %s\n", rc, lin);
							rule(lin, rules, &rc);
						}
					}
					fclose(f);
				} else
					printf("\nCouldn't open rule-file '%s'\n", dp->d_name);
			}
		}
		if (chdir("..") == -1)
			fail("directory change failed");
		closedir(dirp);
	} else
		printf("\nCouldn't open rules directory '%s'\n", dir);

	return rc;	/* rule count */
}

void rule(char *r, exp_tree_t **rules, int* rc)
{
	char* buf;
	token_t* tokens;
	exp_tree_t tree;
	int mc;
	int i;

	extern void fail(char*);

	/* lexing */
	tokens = tokenize(r);

	/* parsing */
	tree = *((parse(tokens)).child[0]);

	/* some preventive checking */
	if (tree.head_type != EQL) {
		printf("\n");
		fail("you asked me to store, as a rule, a non-rule expression :(");
		printout_tree(tree);
		printf("\n");
	}

	rules[(*rc)++] = alloc_exptree(tree);
}



/*
 * The code here interfaces between
 * the stupid JavaScript GUI stuff
 * and the C code responsible for
 * doing the actual algorithmic work.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern char *doit(int yes, char *lin);

extern int count;
int warm = 0;
char* display_div;
int first_only = 0;
extern char *step_desc;

/* This is what the javascript code calls
 * when it wants the C code to do something.
 *
 * target_div specifies where the resulting HTML
 * is to be written to. It gets copied to a global
 * variable called "display_div" which the output
 * module reads.
 *
 * run_main = 0 means only a preview is wanted
 * run_main = 1 means differentiation is wanted
 */
int jsdoit(char *expression, char *target_div, int run_main)
{
	step_desc = malloc(1024 * 1024);
	display_div = malloc(1024);
	display_div[0] = 0;
	strcpy(display_div, target_div);
	extern void do_setup(void);
	extern void do_cleanup(void);

	/* clear step-counter */
	count = 0;

	if (!run_main) {
		/* first_only is a global flag
		 * that specifies that preview
		 * mode is in effect. it causes
		 * the code to bail out early
		 * once the first line has
		 * been printed.
		 */
		first_only = 1;
	} else {
		first_only = 0;
	}

	/* When the preview button is clicked
	 * several times, the code has to make
 	 * sure to do its 'setup' routine only
	 * once, else the preview button crashes
 	 * after a few uses.
	 */
	if (!warm)
		do_setup();
	warm = 1;

	printf("expr: %s\n", expression);

	doit(run_main, expression);

	/* some clean-up */
	free(display_div);
	free(step_desc);

	/*
	 * it is useful to return `count',
	 * since if it is zero, something has
	 * gone wrong and the JS will pop
	 * up a generic error box.
	 */
	return count;
}

/* This hooks to an identically-named JavaScript
 * routine which pops up a message box with the
 * failure message and blocks the generic error
 * box that is shown if the code segfaults.
 */
void fail_alert(char *s) {
	char buf[1024];
	sprintf(buf, "fail_alert('%s')", s);
	emscripten_run_script(buf);
	exit(1);
}


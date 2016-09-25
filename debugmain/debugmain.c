#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char *doit(int yes, char *lin);

char *display_div;
extern int count;
char* step_desc;
int first_only = 0;

/* char *doit(int run_main_program, char *lin) */



void jsdoit(char *expression, char *target_div, int run_main)
{
	display_div = malloc(1024);
	display_div[0] = 0;
	strcpy(display_div, target_div);
	step_desc = malloc(1024 * 1024);

	if (!run_main) {
		count = 0;
		first_only = 1;
	} else {
		first_only = 0;
		count = 0;
	}

	do_setup();
	doit(run_main, expression);
	do_cleanup();

	free(step_desc);
	free(display_div);
}

void main()
{
	char buf[1024];
	int mode = -1;
	printf("command: "); fflush(stdout);
	fgets(buf, 1024, stdin);
	while (!(mode == 0 || mode == 1)) {
		printf("mode: "); fflush(stdout);
		scanf("%d", &mode);
	}
	jsdoit(buf, "derp", mode);
}

void fail_alert(char *s){
	puts(s);
	exit(1);
}


#include "gc.h"

/* General-use routines */

#include <stdlib.h>
#include <stdio.h>

void fail(char* mesg)
{
	/* Have the GUI pop up a message box */
	char buf[1024];
	sprintf(buf, "symdiff has crashed: '%s'\n", mesg);
	fail_alert(buf);
}

void sanity_requires(int exp)
{
	if(!exp)
		fail("that doesn't make any sense");
}


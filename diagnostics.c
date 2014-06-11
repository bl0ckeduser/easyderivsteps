#include "gc.h"

#include "tokenizer.h"
#include "tokens.h"
#include <string.h>

/* 
 * Stripped-down, semi "user-friendly" parsing error,
 * hooks to the JS GUI via fail_alert
 */

void compiler_fail(char *message, token_t *token,
        int in_line, int in_chr)
{
	fail_alert("Syntax is invalid. Please click \"Help\" for a brief syntax guide.");
}

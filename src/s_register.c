/* $Id: s_register.c,v 1.11 2009/04/04 09:08:40 hoche Exp $ */

#include "icb.h"

#include <readline/readline.h>

/* /s_register is like /p, but if you don't give it an
   argument, it will ask you for a password without echoing it. */

int
s_register(ARGV_TCL)
{
    if (argc == 1) {
        regnick(NULL);
    } else if (argc == 2) {
        regnick(argv[1]);
    } else {
        RETURN_ERR_ARGC("?password?");
    }
    return TCL_OK;
}

void
regnick(char *password)
{
    if (!password) {
        extern int _rl_meta_flag;   /* private var, sigh. */

        /* The problem is that readline sets -icrnl in the tty, and
           getpass won't unset it, so <return> doesn't work.

           So, we undo/redo the tty settings.  This is bad if we're not
           actually inside readline, but it's only a minor bad. */

        rl_deprep_term_function();
        password = getpass("Password:");
        rl_prep_term_function(_rl_meta_flag);
    }

    if (*password != '\0') {
        send_command("p", password);
    }
}

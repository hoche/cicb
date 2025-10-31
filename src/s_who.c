/* $Id: s_who.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to list users */

#include "icb.h"

int
s_who(ARGV_TCL)
{
    static char *usage = "usage: s_who long|short [group]";

    /* check the arg count */
    if (argc < 2 || !*argv[1])
        TRETURNERR(usage);

    /* FIXME - right now, only long reports are is valid */
    if (strcmp(argv[1], "long"))
        TRETURNERR("s_who: only long who requests for now");

    if (argc == 3)
        sendcmd("w", argv[2]);
    else
        sendcmd("w", "");
    return (TCL_OK);
}

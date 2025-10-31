/* $Id: s_version.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to request ICB version info */

#include "icb.h"

int
s_version(ARGV_TCL)
{
    static char *usage = "usage: s_version";

    /* check the arg count */
    if (argc != 1)
        TRETURNERR(usage);

    sendcmd("v", "");
    return (TCL_OK);
}

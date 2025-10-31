/* $Id: s_motd.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to print the motd */

#include "icb.h"

int s_motd(ARGV_TCL) {
    static char *usage = "usage: s_motd";

    /* check the arg count */
    if (argc != 1)
        TRETURNERR(usage);

    sendcmd("motd", "");
    return (TCL_OK);
}

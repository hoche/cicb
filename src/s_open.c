/* $Id: s_open.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to send a line to a group */
/* this is included for Tcl purposes, but most text bypasses this */

#include "icb.h"

int s_open(ARGV_TCL) {
    static char *usage = "usage: s_open text ...";

    if (argc < 2 || ! argv[1][0])
        TRETURNERR(usage);

    csendopen(catargs(&argv[1]));
    return (TCL_OK);
}

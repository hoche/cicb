/* $Id: c_rand.c,v 1.6 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to generate a random number */

#include "icb.h"

int c_rand(ARGV_TCL) {
    static char *usage = "usage: c_rand number";
    int range;
    char num[64];

    if (argc != 2 || ! *argv[1])
        TRETURNERR(usage);

    if (safe_atoi(argv[1], &range) != 0 || range == 0)
        TRETURNERR("c_rand: zero or non-numeric argument given");

    snprintf(num, sizeof(num), "%d", (rand() % range) + 1);

    /* pass the number back to the caller */
    Tcl_Return(interp, num, TCL_VOLATILE);

    return (TCL_OK);
}

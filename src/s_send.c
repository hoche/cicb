/* $Id: s_send.c,v 1.8 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to send a line to a group */
/* this is included for Tcl purposes, but most text bypasses this */

#include "icb.h"

int s_send(ARGV_TCL) {
    static char *usage = "usage: s_send text ...";

    if (argc < 2 || ! *argv[1])
        TRETURNERR(usage);

    send_default(catargs(&argv[1]), 0);
    return (TCL_OK);
}

void send_default(char *text, int echoflag) {
    if (gv.personalto && *gv.personalto)
        sendpersonal(gv.personalto, text, echoflag);
    else
        csendopen(text);
}

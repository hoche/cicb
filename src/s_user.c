/* $Id: s_user.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to change user info */

#include "icb.h"

int s_user(ARGV_TCL) {
    static char *usage = "usage: s_user nick|echoback args";

    if (argc < 2 || ! *argv[1])
        TRETURNERR(usage);

    if (strcmp(argv[1], "nick") == 0) {
        if (argc < 3)
            TRETURNERR("usage: s_user nick newnick");

        sendcmd("name", catargs(&argv[2]));
        return (TCL_OK);
    }

    else if (strcmp(argv[1], "echoback") == 0) {
        if (argc < 3)
            TRETURNERR("usage: s_user echoback on|off");
        sendcmd("echoback", catargs(&argv[2]));
        return (TCL_OK);
    }

    else
        TRETURNERR(usage);
}

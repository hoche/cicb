/* $Id: s_group.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */ /* primitive to request group functions */

#include "icb.h"

int s_group(ARGV_TCL) {
    static char *usage =
        "usage: s_group cancel|change|create|invite|pass|remove|status|topic "
        "args";

    if (argc < 2 || ! *argv[1])
        TRETURNERR(usage);

    if (strcmp(argv[1], "cancel") == 0) {
        if (argc != 3)
            TRETURNERR("usage: s_group cancel nick");
        sendcmd("cancel", argv[2]);
        return (TCL_OK);
    }

    if (strcmp(argv[1], "change") == 0) {
        if (argc != 3)
            TRETURNERR("usage: s_group change group");
        sendcmd("g", argv[2]);
        return (TCL_OK);
    }

    if (strcmp(argv[1], "create") == 0) {
        if (argc != 3)
            TRETURNERR("usage: s_group create group");
        sendcmd("g", argv[2]);
        return (TCL_OK);
    }

    if (strcmp(argv[1], "invite") == 0) {
        if (argc != 3)
            TRETURNERR("usage: s_group invite nick");
        sendcmd("invite", argv[2]);
        histput(argv[2]);
        sprintf(mbuf, "%cinvite %s", gv.cmdchar, argv[2]);
        bufferadd(mbuf);
        return (TCL_OK);
    }

    if (strcmp(argv[1], "pass") == 0) {
        if (argc > 3)
            TRETURNERR("usage: s_group pass nick");
        if (argc == 3)
            sendcmd("pass", argv[2]);
        else
            sendcmd("pass", "");
        return (TCL_OK);
    }

    if (strcmp(argv[1], "remove") == 0) {
        if (argc != 3)
            TRETURNERR("usage: s_group remove nick");
        sendcmd("boot", argv[2]);
        return (TCL_OK);
    }

    if (strcmp(argv[1], "status") == 0) {
        if (argc == 2) {
            sendcmd("status", "");
            return (TCL_OK);
        }
        sendcmd("status", catargs(&argv[2]));
        return (TCL_OK);
    }

    if (strcmp(argv[1], "topic") == 0) {
        if (argc < 3)
            TRETURNERR("usage: s_group topic newtopic...");
        sendcmd("topic", argv[2]);
        return (TCL_OK);
    }

    TRETURNERR(usage);
}

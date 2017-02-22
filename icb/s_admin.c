/* $Id: s_admin.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to change user info */

#include "icb.h"

int
s_admin (ARGV_TCL)
{
	static char *usage = "usage: s_admin drop|shutdown|wall args";

	if (argc < 2 || !*argv[1])
		TRETURNERR(usage)

	if (strcmp(argv[1], "drop") == 0) {
		if (argc < 3)
			TRETURNERR("usage: s_admin drop id")
		sendcmd("drop", catargs(&argv[2]));
		return(TCL_OK);
	}

	else if (strcmp(argv[1], "shutdown") == 0) {
		if (argc < 3)
			TRETURNERR("usage: s_admin shutdown when exp")
		sendcmd("shutdown", catargs(&argv[2]));
		return(TCL_OK);
	}

	else if (strcmp(argv[1], "wall") == 0) {
		if (argc < 3)
			TRETURNERR("usage: s_admin wall text")
		sendcmd("wall", catargs(&argv[2]));
		return(TCL_OK);
	}

	else
		TRETURNERR(usage)
}

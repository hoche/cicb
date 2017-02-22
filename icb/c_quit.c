/* $Id: c_quit.c,v 1.7 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to exit icb */

#include "icb.h"

int
c_quit (ARGV_TCL)
{
	static char *usage = "usage: c_quit";

	if (argc != 1) {
		/* if args are wrong, print usage but still quit */
		putl(usage, PL_SCR);
	}
	if (gv.verifyquit) {
		askquit();
		return(TCL_OK);
	}
		
	icbexit();
	/* NOTREACHED */
	return 0;
}

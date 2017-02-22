/* $Id: c_version.c,v 1.9 2009/04/04 09:08:40 hoche Exp $ */

/* primitive to display client version info */
#include "icb.h"

#include <readline/readline.h>

int
c_version (ARGV_TCL)
{
	static char *usage = "usage: c_version";
    char buf[256];

	/* check the arg count */
	if (argc != 1)
		TRETURNERR(usage)

	putl(icb_version, PL_SCR);
	sprintf(buf, "Built with readline %s and TCL %s",
		rl_library_version,
		TCL_PATCH_LEVEL);
	putl(buf, PL_SCR);
	return(TCL_OK);
}

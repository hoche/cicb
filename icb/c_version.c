/* primitive to display client version info */
#include "icb.h"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#endif

int
c_version (ARGV_TCL)
{
	static char *usage = "usage: c_version";
    char buf[BUFSIZ];

	/* check the arg count */
	if (argc != 1)
		TRETURNERR(usage)

	putl(icb_version, PL_SCR);
#ifdef HAVE_READLINE
    snprintf(buf, BUFSIZ, "   readline %s", rl_library_version);
	putl(buf, PL_SCR);
#endif
#ifdef HAVE_TCL
    snprintf(buf, BUFSIZ, "   TCL %s", TCL_PATCH_LEVEL);
	putl(buf, PL_SCR);
#endif

	return(TCL_OK);
}

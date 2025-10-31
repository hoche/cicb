#include "icb.h"
#include <readline/readline.h>

int c_version(ARGV_TCL) {
    static char *usage = "usage: c_version";
    char buf[BUFSIZ];

    /* check the arg count */
    if (argc != 1)
        TRETURNERR(usage);

    putl(icb_version, PL_SCR);
    snprintf(buf, BUFSIZ, "   readline %s", rl_library_version);
    putl(buf, PL_SCR);
    snprintf(buf, BUFSIZ, "   TCL %s", TCL_PATCH_LEVEL);
    putl(buf, PL_SCR);

    return (TCL_OK);
}

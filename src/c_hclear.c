/* primitive to clear the history list */

#include "icb.h"

int c_hclear(ARGV_TCL) {
    static char *usage = "usage: c_hclear";

    if (argc > 1)
        TRETURNERR(usage);

    histclear();
    return TCL_OK;
}

#include "getswitch.h"
#include "icb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* getswitch - parse multicharacter option arguments.
 */

int switchind = 1;
char *switcharg;
static char *ambiguous = "?ambiguous";
static char *unknown = "?unknown";
static char *noarg = "?noarg";

char *getswitch(int argc, char **argv, char **switchv) {
    char *slash;
    char *thisswitch;
    char *retswitch;
    char *arg;
    int length;
    int switcherr = 1;

    if (switchind >= argc)
        return NULL;

    if (argv[switchind][0] != '-')
        return NULL;

    if (argv[switchind][1] == '-')
        return NULL;

    arg = argv[switchind++] + 1;

    thisswitch = NULL;
    retswitch = NULL;
    while (switchv && *switchv) {
        if (thisswitch != retswitch)
            free(thisswitch);

        thisswitch = strdup(*switchv);
        length = strlen(arg);

        if ((slash = strchr(thisswitch, '/')) != NULL) {
            *slash = '\0';
            size_t thisswitch_len = strlen(thisswitch);
            size_t suffix_len = strlen(slash + 1);
            if (thisswitch_len + suffix_len + 1 <= strlen(*switchv) + 1) {
                safe_strncat(thisswitch, slash + 1, strlen(*switchv) + 1);
            }
            length = slash - thisswitch;
        }

        if (! strncmp(arg, thisswitch, length)) {
            if (retswitch) {
                if (switcherr) {
                    fprintf(stderr, "%s: -%s ambiguous.\n", argv[0], arg);
                }
                free(retswitch);
                free(thisswitch);
                switcharg = arg;
                return ambiguous;
            }
            retswitch = thisswitch;
        }
        switchv++;
    }

    if (retswitch != thisswitch) {
        free(thisswitch);
    }

    if (retswitch) {
        length = strlen(retswitch) - 1;
        if (retswitch[length] == ':') {
            retswitch[length] = '\0';
            if (switchind >= argc) {
                if (switcherr) {
                    fprintf(stderr, "%s: -%s: argument expected.\n", argv[0],
                            retswitch);
                }
                switcharg = retswitch;
                return noarg;
            } else
                switcharg = argv[switchind++];
        }
        return retswitch;
    }

    if (switcherr) {
        fprintf(stderr, "%s: -%s: unknown option.\n", argv[0], arg);
    }
    switcharg = arg;
    return unknown;
}

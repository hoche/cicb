/* $Id: oset.c,v 1.26 2009/04/04 09:08:40 hoche Exp $ */

/* primitive to set icb internal variables */

#include "icb.h"

#include <readline/readline.h>

/* struct for variable table */
typedef struct Variable {
    char *name;
    int type;
    char *address;
} VARIABLE;

/* variable types */
#define V_CHAR 1
#define V_BOOLEAN 2
#define V_NONNEG 3
#define V_INT 4
#define V_STRING 5

/* list of usable variables */
VARIABLE vars[] = {{"alert", V_STRING, (char *) &gv.alert},
                   {"asyncread", V_BOOLEAN, (char *) &gv.asyncread},
                   {"autodel", V_BOOLEAN, (char *) &gv.autodel},
                   {"autoreg", V_BOOLEAN, (char *) &gv.autoregister},
                   {"beeps", V_BOOLEAN, (char *) &gv.beeps},
                   {"bufferlines", V_NONNEG, (char *) &gv.bufferlines},
                   {"commandchar", V_CHAR, (char *) &gv.cmdchar},
                   {"colorize", V_BOOLEAN, (char *) &gv.colorize},
                   {"cute", V_BOOLEAN, (char *) &gv.cute},
                   {"editmode", V_STRING, (char *) &gv.editmode},
                   {"groupblanks", V_BOOLEAN, (char *) &gv.groupblanks},
                   {"history", V_NONNEG, (char *) &gv.phistory},
                   {"keepalive", V_NONNEG, (char *) &gv.keepalive},
                   {"mutepongs", V_BOOLEAN, (char *) &gv.mutepongs},
                   {"logfile", V_STRING, (char *) &gv.logfile},
                   {"logreplay", V_BOOLEAN, (char *) &gv.logreplay},
                   {"pagesize", V_INT, (char *) &gv.pagesize},
                   {"pagewidth", V_INT, (char *) &gv.pagewidth},
                   {"pauseonshell", V_BOOLEAN, (char *) &gv.pauseonshell},
                   {"personalto", V_STRING, (char *) &gv.personalto},
                   {"restricted", V_BOOLEAN, (char *) &gv.restricted},
                   {"tabreply", V_BOOLEAN, (char *) &gv.tabreply},
                   {"timedisplay", V_STRING, (char *) &gv.timedisplay},
                   {"timestamp", V_BOOLEAN, (char *) &gv.timestamp},
                   {"turner", V_BOOLEAN, (char *) &gv.colorize},
                   {"urlgrab", V_BOOLEAN, (char *) &gv.urlgrab},
                   {"verifyquit", V_BOOLEAN, (char *) &gv.verifyquit},
                   {NULL, 0, NULL}};

void set_restricted(void) {
    gv.restricted = 1;
    tcl_restrict();
}

int oset(ARGV_TCL) {
    /* static char *usage = "usage: oset var [value]"; */
    int x;

    if (argc == 1) {
        (void) listvars(NULL);
        return (TCL_OK);
    }

    if (argc == 2) {
        if (listvars(argv[1]) < 0) {
            TRETURNERR("oset: no such variable");
        }
        return (TCL_OK);
    }

    /* "restricted" is a special case */
    if (! strcmp(argv[1], "restricted")) {
        if (gv.restricted) {
            TRETURNERR("oset: restricted mode can't be reset");
        }
        set_restricted();
        return (TCL_OK);
    }

    {
        char *args_str = catargs(&argv[2]);
        if (args_str) {
            safe_strncpy(mbuf, args_str, MESSAGE_BUF_SIZE);
        } else {
            mbuf[0] = '\0';
        }
    }

    for (x = 0; vars[x].name != NULL; x++) {
        if (strcmp(argv[1], vars[x].name) == 0) {
            int ret = setvar(interp, vars[x].type, vars[x].address, mbuf);
            if (ret != TCL_OK)
                return (ret);
            else
                break;
        }
    }

    if (vars[x].name == NULL) {
        char buf[BUFSIZ];
        snprintf(buf, BUFSIZ, "oset: no such variable \"%s\"", argv[1]);
        TRETURNERR(buf);
    }

    return (TCL_OK);
}

void printvar(char *name, int type, char *address) {
    int ival;

    switch (type) {
    case V_CHAR:
        sprintf(mbuf, "%-12s  char  '%c'", name, *address);
        break;
    case V_INT:
        memcpy(&ival, address, sizeof(int));
        sprintf(mbuf, "%-12s  int   %d", name, ival);
        break;
    case V_NONNEG:
        memcpy(&ival, address, sizeof(int));
        sprintf(mbuf, "%-12s  nneg  %d", name, ival);
        break;
    case V_BOOLEAN:
        sprintf(mbuf, "%-12s  bool  %s", name,
                *(int *) address ? "true" : "false");
        break;
    case V_STRING:
        sprintf(mbuf, "%-12s  str   \"%s\"", name, *(char **) address);
        break;
    default:
        sprintf(mbuf, "printvar unknown type %d for \"%8s\"", type, name);
        break;
    }
    putl(mbuf, PL_SCR);
}

int listvars(char *name) {
    int x;

    if (name == NULL) {
        putl("VARIABLE      TYPE  VALUE", PL_SCR);
        for (x = 0; vars[x].name != NULL; x++)
            printvar(vars[x].name, vars[x].type, vars[x].address);
        return (0);
    }

    else
        for (x = 0; vars[x].name != NULL; x++)
            if (strcmp(name, vars[x].name) == 0) {
                putl("VARIABLE      TYPE  VALUE", PL_SCR);
                printvar(vars[x].name, vars[x].type, vars[x].address);
                break;
            }

    if (vars[x].name == NULL)
        return (-1);
    else
        return (0);
}

/* perform a sanity check on some variables */
void varsanity() {
    if (gv.pagesize < 0 || gv.pagewidth < 0) {
        getwinsize();
    }

    if (gv.keepalive > 0) {
        /* less than 60 second is too low */
        if (gv.keepalive < 60) {
            putl("Minimum value is 60 seconds. Setting to 60.", PL_SCR);
            gv.keepalive = 60;
        }
        /* set the timer */
        alarm(gv.keepalive);
    }

    if (strcmp(gv.editmode, "emacs") == 0) {
        rl_emacs_editing_mode(1, 0);
    } else {
        rl_vi_editing_mode(1, 0);
        gv.editmode = "vi";
    }

    if (strcmp(gv.alert, "none") != 0 && strcmp(gv.alert, "all") != 0 &&
        strcmp(gv.alert, "personal") != 0)
        gv.alert = "none";
}

int unsetvar(Tcl_Interp *interp, int type, char *address) {
    switch (type) {
    case V_CHAR:
        *address = '\0';
        break;
    case V_STRING: {
        char **s_addr = (char **) address;
        *s_addr = "";
        break;
    }
    case V_INT:
    case V_NONNEG:
    case V_BOOLEAN: {
        int *i_addr = (int *) address;
        *i_addr = 0;
    } break;
    }
    varsanity();
    return (TCL_OK);
}

int ounset(ARGV_TCL) {
    static char *usage = "usage: ounset var";
    int x;

    if (argc != 2) {
        TRETURNERR(usage);
    }

    /* "restricted" is a special case */
    if (! strcmp(argv[1], "restricted")) {
        if (gv.restricted) {
            TRETURNERR("ounset: restricted mode can't be reset");
        }
    }

    for (x = 0; vars[x].name != NULL; x++) {
        if (strcmp(argv[1], vars[x].name) == 0) {
            int ret = unsetvar(interp, vars[x].type, vars[x].address);
            if (ret != TCL_OK)
                return (TCL_ERROR);
            else
                break;
        }
    }

    if (vars[x].name == NULL) {
        char buf[BUFSIZ];
        snprintf(buf, BUFSIZ, "ounset: no such variable \"%s\"", argv[1]);
        TRETURNERR(buf);
    }

    return (TCL_OK);
}

int setvar(Tcl_Interp *interp, int type, char *address, char *s) {
    int tmp;
    char *p;

    switch (type) {
    case V_CHAR:
        *address = *s;
        break;
    case V_INT:
        if (safe_atoi(s, &tmp) != 0) {
            TRETURNERR("oset: invalid integer value");
        }
        memcpy(address, &tmp, sizeof(int));
        break;
    case V_NONNEG:
        if (safe_atoi(s, &tmp) != 0) {
            TRETURNERR("oset: invalid integer value");
        }
        if (tmp < 0) {
            TRETURNERR("oset: only non-negative values allowed");
        }
        memcpy(address, &tmp, sizeof(int));
        break;
    case V_BOOLEAN:
        if (! strncmp(s, "on", 2) || ! strncmp(s, "true", 4) || *s == 'y' ||
            *s == '1') {
            int *i_addr = (int *) address;
            *i_addr = 1;
        } else if (! strncmp(s, "off", 2) || ! strncmp(s, "false", 4) ||
                   *s == 'n' || *s == '0') {
            int *i_addr = (int *) address;
            *i_addr = (int) 0;
        } else {
            TRETURNERR("oset: only boolean values allowed");
        }
        break;
    case V_STRING:
        tmp = strlen(s) + 1;
        if ((p = malloc(tmp)) == NULL) {
            TRETURNERR("oset: not enough memory to set string");
        }
        memcpy(p, s, tmp);
        memcpy(address, &p, sizeof(p));
        break;
    }
    varsanity();
    return (TCL_OK);
}

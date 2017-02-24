/* $Id: parse.c,v 1.20 2009/03/23 06:51:53 hoche Exp $ */

/* parse a line from the user */

#include "icb.h"
#include <ctype.h>
#include <stdarg.h>

#define ARRAY_SIZE(x) (sizeof (x) / sizeof (x[0]))

/* This is similar to Tcl_VarEval, but it treats each string as a separate
   list element, rather than concat-ing the strings together and then
   eval-ing the result. */

static int tcl_execl(Tcl_Interp * interp, ...)
{
    va_list args;
    char *arg;
    const char *argv[10];
    unsigned int argc;
    char *command;
    int status;

    va_start(args, interp);
    argc = 0;
    while ((arg = va_arg(args, char *)) != NULL) {
        if (ARRAY_SIZE(argv) <= argc) {
            Tcl_AppendResult(interp, "Too many args to tcl_execl");
            return TCL_ERROR;
        }
        argv[argc++] = arg;
    }
    va_end(args);

    command = Tcl_Merge(argc, argv);
    status = Tcl_Eval(interp, command);
    Tcl_Free(command);
    return status;
}

static int do_command(Tcl_Interp * interp, char *line)
{
    int status;
    char *cmd = line;
    char *args;

    /* find start of command name */
    while (*cmd && isspace(*cmd))
        ++cmd;

    /* find end of command name */
    args = cmd;
    while (*args && !isspace(*args))
        ++args;

    /* split the string there. */
    if (*args) {
        *args++ = '\0';
    }

    /* ignore null commands. */
    if (*cmd == '\0') {
        return TCL_OK;
    }

    /* well, I was going to write code to do the equivalent of
       [info commands ${cmd}*] in C so that abbrev lookup still works if
       the user defines their own proc info, but tcl actually doesn't
       publish a C interface that lets you do that.  Oh well.  This will
       have to be handled some other way. */

    status = tcl_execl(interp, "icb_command_v1", cmd, args, (char *)NULL);

    /* are we supposed to just send the command to the server? */
    if (status == TCL_CONTINUE) {
        send_command(cmd, args);
        status = TCL_OK;
    }

    return status;
}

void parse(char *line)
{
    static int error_mode = 0;
    int status = TCL_OK;
    char *s = line;

    /* is it a public message? */

    if (*s != gv.cmdchar || *++s == gv.cmdchar) {
        send_default(s, 1);
        error_mode = 0;
        return;
    }

    putl(line, PL_LOG);

    if (!gv.restricted && *s == '!') {
        /* shell escape */
        status = tcl_execl(interp, "c_shell", s + 1, (char *)NULL);
    } else if (!gv.restricted && (strncmp(s, "tcl", 3) == 0)
               && (s[3] == '\0' || isspace(s[3]))) {
        /* literal tcl */
        status = Tcl_Eval(interp, s + 4);
    } else {
        /* otherwise it's a command of some sort. */
        status = do_command(interp, s);
    }

    if (status == TCL_OK) {
        /* We succeeded, so print the return value. */
        char *result = (char *)Tcl_GetStringResult(interp);
        if (*result != '\0') {
            putl(result, PL_SL);
        }
        error_mode = 0;

    } else {
        /* Report an error. */
        char *errmsg = "";
        char *c0 = printcolor(ColERROR, ColSANE);
        char *c1 = printcolor(ColSANE, ColSANE);

        if (error_mode == 0) {
            /* Most errors are simple, so do a short error message. */
            error_mode = 1;
            errmsg = (char *)Tcl_GetStringResult(interp);
        } else if (error_mode == 1) {
            error_mode = 2;
            errmsg = "Hi.  Having problems?  Here's a verbose traceback:";
        }

        sprintf(mbuf, "%s[=Tcl Error=] %s%s", c0, errmsg, c1);
        putl(mbuf, PL_SL);

        /* If we had an error before, the user's probably repeating a
           nontrivial error, so give more info.  Don't colorize it, since
           it can easily overrun mbuf. */
        if (error_mode == 2) {
            errmsg =
                (char *)Tcl_GetVar2(interp, "errorInfo", NULL, TCL_GLOBAL_ONLY);
            if (errmsg != NULL) {
                putl(errmsg, PL_SL);
            }
            sprintf(mbuf, "%s[=End Traceback=]%s", c0, c1);
            putl(mbuf, PL_SL);
        }
    }
}

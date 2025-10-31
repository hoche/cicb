/* $Id: parse.c,v 1.21 2013/07/05 06:16:27 hoche Exp $ */

/* parse a line from the user */

#include "icb.h"
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define TCL_CONTINUE_ERROR_MSG "invoked \"continue\" outside of a loop"
#define TCL_IGNORE 666

/* C command dispatch table - maps command names to C handler functions */
typedef struct {
    const char *name;
    int (*handler)(ARGV_TCL);
} cmd_handler_t;

static cmd_handler_t c_cmd_handlers[] = {{"c_color", c_color},
                                         {"c_hclear", c_hclear},
                                         {"c_hdel", c_hdel},
                                         {"c_help", c_help},
                                         {"c_hush", c_hush},
                                         {"c_log", c_log},
                                         {"c_quit", c_quit},
                                         {"c_rand", c_rand},
                                         {"c_replay", c_replay},
                                         {"c_shell", c_shell},
                                         {"c_time", c_time},
                                         {"c_usage", c_usage},
                                         {"c_version", c_version},
                                         {"color", c_color},
                                         {"oset", oset},
                                         {"ounset", ounset},
                                         {"s_admin", s_admin},
                                         {"s_beep", s_beep},
                                         {"s_chpw", s_chpw},
                                         {"s_group", s_group},
                                         {"s_motd", s_motd},
                                         {"s_open", s_open},
                                         {"s_personal", s_personal},
                                         {"s_register", s_register},
                                         {"s_send", s_send},
                                         {"s_user", s_user},
                                         {"s_version", s_version},
                                         {"s_who", s_who},
                                         {"time", c_time},
                                         /* Common abbreviations and aliases */
                                         {"who", s_who},
                                         {"w", s_who},
                                         {"help", c_help},
                                         {"h", c_help},
                                         {"quit", c_quit},
                                         {"q", c_quit},
                                         {"beep", s_beep},
                                         {"b", s_beep},
                                         {"log", c_log},
                                         {"display", c_replay},
                                         {"replay", c_replay},
                                         {"d", c_replay},
                                         {"version", s_version},
                                         {"v", s_version},
                                         {"motd", s_motd},
                                         {"register", s_register},
                                         {"p", s_register},
                                         {"nick", s_user},
                                         {"n", s_user},
                                         {"group", s_group},
                                         {"g", s_group},
                                         {"send", s_send},
                                         {"message", s_personal},
                                         {"m", s_personal},
                                         {NULL, NULL}};

/* Find a C command handler for the given command name.
   Returns NULL if not found. Handles exact matches and unique abbreviations. */
static cmd_handler_t *find_c_handler(const char *cmd) {
    cmd_handler_t *found = NULL;
    int matches = 0;
    size_t cmd_len = strlen(cmd);

    for (cmd_handler_t *h = c_cmd_handlers; h->name != NULL; h++) {
        if (strcmp(h->name, cmd) == 0) {
            /* Exact match */
            return h;
        }
        if (strncmp(h->name, cmd, cmd_len) == 0) {
            /* Abbreviation match */
            if (found == NULL) {
                found = h;
                matches = 1;
            } else {
                matches++;
            }
        }
    }

    /* Return handler only if unique abbreviation */
    return (matches == 1) ? found : NULL;
}

/* Split arguments string into argv array for C handlers.
   Returns allocated array that caller must free. */
static char **split_args(const char *args_str, int *argc_out) {
    char **argv = NULL;
    int argc = 0;
    int max_args = 32;
    const char *p = args_str;
    char *arg_buf = NULL;
    size_t arg_buf_size = 0;

    if (args_str == NULL || *args_str == '\0') {
        *argc_out = 0;
        return NULL;
    }

    argv = malloc(max_args * sizeof(char *));
    if (argv == NULL) {
        *argc_out = 0;
        return NULL;
    }

    /* Allocate buffer for parsing */
    arg_buf_size = strlen(args_str) + 1;
    arg_buf = malloc(arg_buf_size);
    if (arg_buf == NULL) {
        free(argv);
        *argc_out = 0;
        return NULL;
    }

    /* Skip leading whitespace */
    while (*p && isspace((unsigned char) *p))
        p++;

    while (*p && argc < max_args - 1) {
        char *arg = arg_buf;
        int in_quotes = 0;

        /* Parse one argument */
        while (*p) {
            if (*p == '\\' && p[1]) {
                /* Escape sequence */
                *arg++ = *++p;
                p++;
            } else if (*p == '"' || *p == '\'') {
                /* Quote handling */
                char quote = *p++;
                in_quotes = ! in_quotes;
                while (*p && (*p != quote || in_quotes)) {
                    if (*p == '\\' && p[1] && quote == '"') {
                        *arg++ = *++p;
                        p++;
                    } else if (*p != quote) {
                        *arg++ = *p++;
                    } else {
                        p++;
                        in_quotes = 0;
                    }
                }
                if (*p == quote)
                    p++;
            } else if (isspace((unsigned char) *p) && ! in_quotes) {
                /* End of argument */
                break;
            } else {
                *arg++ = *p++;
            }
        }
        *arg = '\0';

        /* Store argument */
        if (arg_buf[0] != '\0' || in_quotes) {
            argv[argc] = strdup(arg_buf);
            if (argv[argc] == NULL) {
                /* Out of memory - free what we have */
                for (int i = 0; i < argc; i++)
                    free(argv[i]);
                free(argv);
                free(arg_buf);
                *argc_out = 0;
                return NULL;
            }
            argc++;
        }

        /* Skip whitespace */
        while (*p && isspace((unsigned char) *p))
            p++;
    }

    argv[argc] = NULL;
    *argc_out = argc;
    free(arg_buf);
    return argv;
}

/* Check if TCL has a handler for the given command.
   Returns 1 if TCL handler exists, 0 otherwise. */
static int tcl_has_handler(const char *cmd) {
    if (interp == NULL)
        return 0;

    Tcl_CmdInfo cmdinfo;
    /* Check for exact match */
    if (Tcl_GetCommandInfo(interp, cmd, &cmdinfo)) {
        return 1;
    }

    /* Do not attempt Tcl-side abbreviation matching to avoid unsafe eval. */
    return 0;
}

/* Call TCL handler if available. Returns status code. */
static int call_tcl_handler(const char *cmd, const char *args_str) {
    if (interp == NULL)
        return TCL_ERROR;

    int status;
    Tcl_CmdInfo cmdinfo;
    const char *actual_cmd = cmd;

    /* Check for exact match only (no Tcl-side abbreviation resolution). */
    if (! Tcl_GetCommandInfo(interp, cmd, &cmdinfo)) {
        return TCL_ERROR;
    }

    /* Build command string */
    if (args_str && *args_str) {
        char *cmd_str;
        size_t len = strlen(actual_cmd) + strlen(args_str) + 2;
        cmd_str = malloc(len);
        if (cmd_str == NULL)
            return TCL_ERROR;
        snprintf(cmd_str, len, "%s %s", actual_cmd, args_str);
        status = Tcl_Eval(interp, cmd_str);
        free(cmd_str);
    } else {
        status = Tcl_Eval(interp, actual_cmd);
    }

    return status;
}

/* This is similar to Tcl_VarEval, but it treats each string as a separate
   list element, rather than concat-ing the strings together and then
   eval-ing the result. */

static int tcl_execl(Tcl_Interp *interp, ...) {
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
            va_end(args);
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

static int do_command(Tcl_Interp *interp, char *line) {
    int status = TCL_OK;
    char *cmd = line;
    char *args = NULL;
    char *args_copy = NULL;

    /* find start of command name */
    while (*cmd && isspace((unsigned char) *cmd))
        ++cmd;

    /* find end of command name */
    args = cmd;
    while (*args && ! isspace((unsigned char) *args))
        ++args;

    /* split the string there. */
    if (*args) {
        *args++ = '\0';
        /* Skip whitespace after command */
        while (*args && isspace((unsigned char) *args))
            ++args;
        if (*args == '\0')
            args = NULL;
    } else {
        args = NULL;
    }

    /* ignore null commands. */
    if (*cmd == '\0') {
        return TCL_OK;
    }

    /* Make a copy of args for safety since we might modify it */
    if (args && *args) {
        args_copy = strdup(args);
        if (args_copy == NULL)
            return TCL_ERROR;
    }

    /* Inverted dispatch: Check TCL handler first (if TCL is available),
       then C handler, then send to server. */

    /* Step 1: Check if TCL has a handler for this command */
    if (tcl_has_handler(cmd)) {
        /* TCL handler exists - use it */
        status = call_tcl_handler(cmd, args_copy);
        if (args_copy)
            free(args_copy);
        return status;
    }

    /* Step 2: Check if C has a handler for this command */
    cmd_handler_t *c_handler = find_c_handler(cmd);
    if (c_handler != NULL) {
        /* C handler exists - use it */
        int argc;
        char **argv = split_args(args_copy, &argc);

        if (argv == NULL && args_copy != NULL && *args_copy != '\0') {
            /* Error splitting args, but we have a handler */
            /* Try with minimal args: cmd + args string */
            char *argv_buf[3];
            argv_buf[0] = (char *) cmd;
            argv_buf[1] = args_copy;
            argv_buf[2] = NULL;
            status = c_handler->handler(NULL, interp, 2, argv_buf);
            if (args_copy)
                free(args_copy);
            return status;
        }

        /* Build argv array for handler */
        char **handler_argv;
        int handler_argc = argc + 1;
        handler_argv = malloc((handler_argc + 1) * sizeof(char *));
        if (handler_argv == NULL) {
            if (argv) {
                for (int i = 0; i < argc; i++)
                    free(argv[i]);
                free(argv);
            }
            if (args_copy)
                free(args_copy);
            return TCL_ERROR;
        }

        handler_argv[0] = (char *) cmd;
        if (argv) {
            for (int i = 0; i < argc; i++)
                handler_argv[i + 1] = argv[i];
        }
        handler_argv[handler_argc] = NULL;

        status = c_handler->handler(NULL, interp, handler_argc, handler_argv);

        /* Free the argv array and the duplicated strings */
        if (argv) {
            for (int i = 0; i < argc; i++)
                free(argv[i]);
            free(argv);
        }
        free(handler_argv);
        if (args_copy)
            free(args_copy);
        return status;
    }

    /* Step 3: No handler found - send command to server */
    if (args_copy)
        free(args_copy);
    send_command(cmd, args);
    return TCL_OK;
}

void parse(char *line) {
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

    if (! gv.restricted && *s == '!') {
        /* shell escape */
        if (interp != NULL) {
            status = tcl_execl(interp, "c_shell", s + 1, (char *) NULL);
        } else {
            /* TCL not available - handle shell escape directly */
            cmd_handler_t *handler = find_c_handler("c_shell");
            if (handler != NULL) {
                char *argv_buf[3];
                argv_buf[0] = "c_shell";
                argv_buf[1] = s + 1;
                argv_buf[2] = NULL;
                status = handler->handler(NULL, NULL, 2, argv_buf);
            } else {
                snprintf(mbuf, MESSAGE_BUF_SIZE,
                         "%s[=Error=] Shell escape not available%s",
                         printcolor(ColERROR, ColSANE),
                         printcolor(ColSANE, ColSANE));
                putl(mbuf, PL_SL);
                status = TCL_ERROR;
            }
        }
    } else if (! gv.restricted && (strncmp(s, "tcl", 3) == 0) &&
               (s[3] == '\0' || isspace((unsigned char) s[3]))) {
        /* literal tcl */
        if (interp != NULL) {
            status = Tcl_Eval(interp, s + 4);
        } else {
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=Error=] TCL not available%s",
                     printcolor(ColERROR, ColSANE),
                     printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_SL);
            status = TCL_ERROR;
        }
    } else {
        /* otherwise it's a command of some sort. */
        status = do_command(interp, s);
    }

    if (status == TCL_OK) {
        /* We succeeded, so print the return value. */
        if (interp != NULL) {
            char *result = (char *) Tcl_GetStringResult(interp);
            if (*result != '\0') {
                putl(result, PL_SL);
            }
        }
        error_mode = 0;

    } else if (status == TCL_IGNORE) {
        /* do nothing */
        error_mode = 0;
    } else {
        /* Report an error. */
        char *errmsg = "";
        char *c0 = printcolor(ColERROR, ColSANE);
        char *c1 = printcolor(ColSANE, ColSANE);

        if (interp != NULL) {
            if (error_mode == 0) {
                /* Most errors are simple, so do a short error message. */
                error_mode = 1;
                errmsg = (char *) Tcl_GetStringResult(interp);
            } else if (error_mode == 1) {
                error_mode = 2;
                errmsg = "Hi.  Having problems?  Here's a verbose traceback:";
            }

            snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=Tcl Error=] %s%s", c0, errmsg,
                     c1);
            putl(mbuf, PL_SL);

            /* If we had an error before, the user's probably repeating a
               nontrivial error, so give more info.  Don't colorize it, since
               it can easily overrun mbuf. */
            if (error_mode == 2) {
                errmsg = (char *) Tcl_GetVar2(interp, "errorInfo", NULL,
                                              TCL_GLOBAL_ONLY);
                if (errmsg != NULL) {
                    putl(errmsg, PL_SL);
                }
                snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=End Traceback=]%s", c0,
                         c1);
                putl(mbuf, PL_SL);
            }
        } else {
            /* TCL not available - simple error message */
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=Error=] Command failed%s", c0,
                     c1);
            putl(mbuf, PL_SL);
            error_mode = 0;
        }
    }
}

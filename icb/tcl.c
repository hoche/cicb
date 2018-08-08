/* $Id: tcl.c,v 1.26 2009/03/23 06:41:26 hoche Exp $ */

/* TCL interface */

#include "icb.h"

Tcl_Interp* interp = NULL;

static char* tclvar_icb_sys_datadir = ICB_SYS_DATADIR;


/*
  icb_send_command COMMAND ARGSTRING
  Sends a command to the icb server.
*/

static int
tclproc_icb_send_command (ARGV_TCL)
{
    if (argc != 3) {
	RETURN_ERR_ARGC ("COMMAND ARGSTRING");
    }
    send_command (argv[1], argv[2]);
    return TCL_OK;
}


/*
  Run the tcl script in STRING.  NAME is used when printing errors.
*/

static void
tcl_eval_string (char* string, char* name)
{
    if (Tcl_Eval (interp, string) != TCL_OK) {
	printf ("%s failed:\n", name);
	printf ("%s\n", Tcl_GetVar (interp, "errorInfo", 0));
    }
}


/*
  List of tcl commands to install.
*/

typedef struct {
    char* tc_name;
    int (*tc_func)(ARGV_TCL);
} tcl_command_t;

static tcl_command_t tcl_commands[] = {
    { "c_color", c_color },
    { "c_hclear", c_hclear },
    { "c_hdel", c_hdel },
    { "c_help", c_help },
    { "c_hush", c_hush },
    { "c_log", c_log },
    { "c_quit", c_quit },
    { "c_rand", c_rand },
    { "c_replay", c_replay },
    { "c_shell", c_shell },
    { "c_time", c_time },
    { "c_usage", c_usage },
    { "c_version", c_version },
    { "color", c_color },
    { "icb_send_command", tclproc_icb_send_command },
    { "oset", oset },
    { "ounset", ounset },
    { "s_admin", s_admin },
    { "s_beep", s_beep },
    { "s_chpw", s_chpw },
    { "s_group", s_group },
    { "s_motd", s_motd },
    { "s_open", s_open },
    { "s_personal", s_personal },
    { "s_register", s_register },
    { "s_send", s_send },
    { "s_user", s_user },
    { "s_version", s_version },
    { "s_who", s_who },
    { "time", c_time },
    { NULL, NULL },
};


/*
  Create the tcl interpreter and run the tcl_init script
*/

static char* tcl_init_string =
#include "tcl_init.string"
;

void
tcl_init()
{
    tcl_command_t* tc;

    interp = Tcl_CreateInterp ();
    if (interp == NULL) {
	abort ();
    }

    for (tc = tcl_commands; tc->tc_name != NULL; ++ tc) {
	Tcl_CreateCommand (interp, tc->tc_name, (Tcl_CmdProc*)tc->tc_func, NULL, NULL);
    }

    Tcl_LinkVar (interp, "personalhilite", (char*) &personalhilite,
		 TCL_LINK_STRING);

    Tcl_LinkVar (interp, "myServer", (char*) &myserver,
		 TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    Tcl_LinkVar (interp, "myGroup", (char*) &mygroup,
		 TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    Tcl_LinkVar (interp, "myNick", (char*) &mynick,
		 TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    Tcl_LinkVar (interp, "myHost", (char*) &myhost,
		 TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    Tcl_LinkVar (interp, "icb_sys_datadir", (char*) &tclvar_icb_sys_datadir,
		 TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    tcl_eval_string (tcl_init_string, "builtin tcl_init script");
}


/*
  Run the tcl_connected script.  This is supposed to happen after the icb
  server finishes handshaking with us.
*/

static char* tcl_connected_string =
#include "tcl_connected.string"
;

void
tcl_connected (void)
{
    tcl_eval_string (tcl_connected_string, "builtin tcl_connected script");
}


void
tcl_restrict()
{
    /* delete commands that might shellout or read or write files */
    Tcl_DeleteCommand(interp, "exec");
    Tcl_DeleteCommand(interp, "print");
    Tcl_DeleteCommand(interp, "file");
    Tcl_DeleteCommand(interp, "source");
}

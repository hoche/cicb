#pragma once

#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <unistd.h>

#include "protocol.h"

/* generic message buffer size */
/* this needs to be bigger than PACKET_BUF_SIZE defined in protocol.h */
#define MESSAGE_BUF_SIZE 2048 


/* command usage information */

typedef struct Usage {
	char name[9];
	char type;
	char args[128];
	char usage[128];
	struct Usage *prev, *next;
} USAGE;

/* special global variables */

typedef struct {
	char cmdchar;		/* character to prefix a command */
	int verifyquit;		/* verify a quit always */
	int pauseonshell;	/* make user hit ret on shell return */
	int interactive;	/* nonzero if interactive session */
	int pagesize;		/* lines per page */
	int pagewidth;		/* columns per page */
	int phistory;		/*  */
	int bufferlines;	/* lines in the review buffer */
	int groupblanks;	/* print a blank before each group */
	char *logfile;		/* name of session log file */
	int restricted;		/* nonzero if in restricted mode */
	char *timedisplay;	/* how time is displayed */
	int beeps;		/* whether beeps are audible */
	char *editmode;		/* input editing mode */
	int tabreply;		/* should replies go into the tab history? */
	int cute;		/* cute messages when a blank line is entered */
	int autodel;		/* automatically delete nicks from the tab
			        history that produce a "not logged on" error */
	char *personalto;	/* 'permanent' personal message recipient */
	int logreplay;		/* should replays go into the log by default? */
	int keepalive;		/* number of seconds between sending keepalive packets */
	int mutepongs;	    /* silence pong packets (used in keepalives) */
	char *alert;		/* alert mode */
	int autoregister;	/* prompt for password when [=Register=] msg
				   received? */
	int asyncread;		/* don't pause output while user is typing */
	int colorize;		/* display colored output */
	int timestamp;		/* display timestamps in messages */
	int urlgrab;		/* capture URLs from incoming messages */
} GLOBS;

/* info on the user's tty */

typedef struct {
	int rows;
	int cols;
} TTYINFO;

/* a linked list of strings */

typedef struct Strlist {
	struct Strlist *next, *prev;
	char str[1];
} STRLIST;

/* misc */

#define CMDCHAR		'/'	/* default char for prefixing commands */
#define PERMCMDCHAR	'\\'	/* like above, but can't be changed */

/* putl (putline) flags */

#define PL_SCR		1	/* text may go to screen */
#define PL_LOG		2	/* text may go to session log */
#define PL_BUF		4	/* text may go to review buffer */
#define PL_TS		8	/* timestamp the event */
#define PL_SL		(PL_SCR | PL_LOG | PL_TS)
#define PL_ALL		(PL_SCR | PL_LOG | PL_BUF | PL_TS)

/* useful defs for modules using TCL code */

#define TRETURNERR(x) { Tcl_ResetResult(interp); Tcl_SetResult(interp, x, TCL_VOLATILE); return TCL_ERROR; }

/* most functions get interp as an arg.  some need global interp, sigh. */

extern Tcl_Interp* interp;

/* ANSI C arg list for C functions that implement Tcl commands
   (string-based) */

#define ARGV_TCL \
	ClientData clientData, Tcl_Interp* interp, int argc, char** argv

/* Macros for returning warnings and errors. */

#define RETURN_WARNING(MSG) {						\
	Tcl_AppendResult (interp, argv[0], ": ", (MSG), (char*)NULL);	\
	return TCL_OK;						\
}

#define RETURN_ERROR(MSG) {						\
	Tcl_AppendResult (interp, argv[0], ": ", (MSG), (char*)NULL);	\
	return TCL_ERROR;						\
}

/* Macro for returning a standard Tcl "wrong # of args" error. */

#define RETURN_ERR_ARGC(ARGMSG) {					\
	Tcl_AppendResult (interp, "wrong # args: should be \"",		\
			  argv[0], " ", (ARGMSG), "\"", (char*)NULL);	\
	return TCL_ERROR;						\
}

/* Cope with pre-object tcl */

#if TCL_MAJOR_VERSION < 8
# define Tcl_GetStringResult(interp) ((interp)->result)
#endif

#define MAX_HOSTLEN 256
#define MAX_PASSLEN 256

#define PERSONALSL ".icbservrc"

extern int port_fd;		/* file descriptor for our server port */
extern char nick[MAX_NICKLEN+1];
extern char group[MAX_NICKLEN+2]; /* to allow room for @user */
extern char pass[MAX_PASSLEN+1];

struct Cbuf {
    char buf[PACKET_BUF_SIZE];
    char *rptr; /* pointer to location for next read */
    char new;   /* set to 1 if next read is start of new data packet */
    unsigned char size;
    unsigned char remain;   /* packet characters remaining to be read */
};

struct server
{
	char name[MAX_HOSTLEN], host[MAX_HOSTLEN];
	int port;
};

#define COLORSIZE 12	/* how many chars can make an ansi sequence */


struct COLORTABLE {
	char color[COLORSIZE];	/* this color's ansi sequence */
	int defined;		/* has color been set? */ 
};

typedef enum { ColSANE, 
       ColNORMAL, 
       ColNOTICE, 
       ColWARNING, 
       ColERROR, 
       ColNICKNAME,
       ColADDRESS, 
       ColIDLETIME, 
       ColLOGINTIME, 
       ColPERSFROM, 
       ColPERSONAL, 
       ColBEEP, 
       ColMOD, 
       ColUNREG, 
       ColABRKT, 
       ColSBRKT, 
       ColPBRKT, 
       ColWHEAD,
       ColWSUB, 
       ColMORE, 
       ColSTATUS, 
       ColPERSHILITE, 
       ColPERSFROMHILITE,
       ColTIMESTAMP
};


#include "externs.h"
#include "protos.h"


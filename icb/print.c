/* $Id: print.c,v 1.25 2009/04/04 09:08:40 hoche Exp $ */

/* screen and logfile printing routines */

#include "icb.h"

#include <readline/readline.h>


extern FILE *logfp;

/* write a message to various output devices
 *
 * s should be a null-terminated c-style string
 * flags are specified by or'ing the following values
 *
 *     PL_SCR    write to the screen, doing paging if necessary
 *     PL_LOG    write to the logfile
 *     PL_BUF    write to the review buffer
 */

void
putl(s, flags)
char *s;
int flags;
{
	char printbuf[128];
	char *timestamp = NULL;
	int pagesize = 0;

	if (continued) {
		linenumber = 0;
		continued = 0;
	}

	if (gv.timestamp)
		timestamp = gettimestamp();

	if (flags & PL_SCR) {
		/* if paging in effect, do paging */

		/* zero or a positive number for gv.pagesize indicates that
		 * the user set it explicity via "oset pagesize".
		 * zero indicates that paging is off.
		 * negative number indicates that we're using SIGWINCH and
		 * and ioctl to dynamically detect the screensize.
		 * either way, we have to set it back to positive to use it
		 * here.
		 */
		if (gv.pagesize >= 0 )
			pagesize = gv.pagesize;
		else 
			pagesize = -gv.pagesize;

		if (pagesize > 0 && (++linenumber >= (pagesize-1) )) {
			sprintf(printbuf,
				"%s[=More=]%s",
				printcolor(ColMORE, ColNOTICE),
				printcolor(ColSANE, ColSANE)); /*COLOR*/
			pauseprompt(printbuf, 1, 0, 1, " ");
			linenumber = 0;
		}

		/* write to the screen */
		if (gv.timestamp && (flags & PL_TS))
			write(1, timestamp, strlen(timestamp));
		write(1, s, strlen(s));
		write(1, "\r\n", 2);
	}

	/* put line into session log */
	if ((flags & PL_LOG) && logfp != NULL) {
		if (gv.timestamp && (flags & PL_TS))
			fputs(timestamp, logfp);
		fputs(s, logfp);
		putc('\n', logfp);
		fflush(logfp);
	}

	/* add to review buffer */
	if ((flags & PL_BUF) && gv.bufferlines)
		bufferadd(s);
}

/*

 pl_log ("notice", "[=Hush=]", "%{nick:%s} added to hush list", hp->str);

 pl_scr
 pl_log
 pl_buf

 screen only
 screen + log
 screen + log + replay

*/

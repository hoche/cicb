/* $Id: s_beep.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to send a personal message */

#include "icb.h"

int
s_beep (ARGV_TCL)
{
	static char *usage = "usage: s_beep nick";

	/* check the arg count */
	if (argc != 2 || !*argv[1])
		TRETURNERR(usage)

	sendcmd("beep", argv[1]);
	histput(argv[1]);
	sprintf(mbuf,"%cbeep %s",gv.cmdchar,argv[1]);
	bufferadd(mbuf);
	return(TCL_OK);
}

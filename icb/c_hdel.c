/* $Id: c_hdel.c,v 1.6 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to delete entries from the nickname history */

#include "icb.h"

int
c_hdel (ARGV_TCL)
{
	if (argc == 1 || !argv[1])
	{
		TRETURNERR("c_hdel: you must specify a nickname with this command.");
	}
	else
	{
		histdel(argv[1]);
		return(TCL_OK);
	}
}

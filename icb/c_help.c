/* $Id: c_help.c,v 1.7 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to display some help */

#include "icb.h"

extern char *helpkeys[], *helptopics[], **helptexts[];

/* list the table of contents for help */

static void
helptoc()
{
	int x;

	putl("[=Help table of contents=]", PL_SL);
	putl("", PL_SL);
	putl("Help is available for the following subjects:", PL_SL);
	putl("", PL_SL);

	for (x = 0; helpkeys[x] != 0; x++) {
		sprintf(mbuf, "%-16s %s", helpkeys[x], helptopics[x]);
		putl(mbuf, PL_SL);
	}

	putl("", PL_SL);
	sprintf(mbuf,
	 "To get help on a particular subject: %chelp subject", gv.cmdchar);
	putl(mbuf, PL_SL);
	sprintf(mbuf,
	 "To get a list of commands: %ccommands", gv.cmdchar);
	putl(mbuf, PL_SL);
	putl("", PL_SL);
	putl("[=End of help=]", PL_SL);
}


/* get help on a particular subject */
/* returns 0 upon success */
/* returns -1 if no help for given subject */

int
helpme(subject)
char *subject;
{
	int x;
	char **text;

	for (x = 0; helpkeys[x] != 0; x++)
		if (strcasecmp(subject, (char *) helpkeys[x]) == 0) {
			text = (char **)helptexts[x];
			sprintf(mbuf, "[=Help for %s=]", subject);
			putl(mbuf, PL_SL);
			putl("", PL_SL);
			for(x = 0; text[x] != NULL; x++)
				putl(text[x], PL_SL);
			putl("", PL_SL);
			putl("[=End of help=]", PL_SL);
			return(0);
		}
	return(-1);
}


int
c_help (ARGV_TCL)
{
	static char *usage = "usage: c_help [subject]";

	if (argc == 1 || !*argv[1] )
	  {
	    helptoc();
	    return(TCL_OK);
	  }	
	else if (argc == 2)
	  {
	    if (strchr(argv[1], ' '))
	      RETURN_WARNING(usage)
	    else if (helpme(argv[1]) < 0)
	      RETURN_WARNING("no help for that subject")
	    else
	      return(TCL_OK);
	  }
	else
	    RETURN_WARNING(usage)
}


/* $Id: s_person.c,v 1.14 2003/07/07 01:42:16 felixlee Exp $ */

/* primitive to send a personal message */

#include "icb.h"
#include "getswitch.h"

extern int _rl_last_command_was_kill;

int
s_personal (ARGV_TCL)
{
	static char *usage = "usage: s_personal [-permanent|-clear] nick message ...";
	static char *switches[] = { "permanent", "clear", NULL };
	static char *personalto = NULL;
	char nick[MAX_NICKLEN+1];
	int permanent = 0;
	int clearflag = 0;
	char *sw;

	/* check for switches */
	switchind = 1;
	while ((sw = getswitch(argc, argv, switches)) != 0)
	{
		switch (sw[0])
		{
			case 'p':
				if (clearflag)
				{
					TRETURNERR("Only one of -permanent or -clear may be specified.");
				}
				permanent = 1;
				break;

			case 'c':
				if (permanent)
				{
					TRETURNERR("Only one of -permanent or -clear may be specified.");
				}
				clearflag = 1;
				break;
		}
	}

	/* check the arg count */
	if ((argc-switchind+1 < 2) && (!clearflag))
	{
		TRETURNERR(usage)
	}

	if (permanent)
	{
		if (personalto)
			free(personalto);
		personalto = (char *)malloc(strlen(argv[switchind])+1);
		strcpy(personalto, argv[switchind]);
		gv.personalto = personalto;
		if (argc-switchind+1==2)
			return TCL_OK;
	}

	if (clearflag)
	{
		gv.personalto = "";
		if (argc-switchind+1<=2)
			return TCL_OK;
	}

	if (!argv[switchind])
		return TCL_ERROR;

	strncpy(nick,argv[switchind],MAX_NICKLEN);
	nick[MAX_NICKLEN]='\0';
	sendpersonal(nick,catargs(&argv[switchind+1]),0);
	return(TCL_OK);
}


void
sendpersonal(char *nick, char *text, int echoflag)
{
	char *buf;
	int pl_flags;

	switch (echoflag)
	{
		case 0:
			pl_flags = PL_BUF;
			break;

		case -1: 
			pl_flags = 0;
			break;

		default:
			pl_flags = PL_BUF | PL_SCR | PL_TS;
	}

	if (!nick)
		return;

	if (!text || !*text) {
		/* This used to prompt for a line then send it.  Disable
                   until I resolve an issue with getline().  Eventually
                   I want to make this do a private-send mode. */
		return;
	}

	histput(nick);

	buf = (char *)malloc(strlen(nick)+strlen(text)+5);
	if (!buf) {
		sprintf(mbuf,
			"%s[=Error=] No memory to build packet%s",
			printcolor(ColERROR, ColSANE),
			printcolor(ColSANE, ColSANE));
		putl(mbuf,PL_SCR|PL_TS);
		return;
	}

	sprintf(buf,"%cm %s %s", gv.cmdchar, nick, text);
	putl(buf, pl_flags);
	sendcmd("m", buf+3);

	free (buf);
}

/* $Id: c_hush.c,v 1.10 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to hush abusive users */

#include "icb.h"

STRLIST *hushhead = NULL, *hushtail = NULL;

int
hushadd(Tcl_Interp *interp, char *nick)
{
	STRLIST *hp;

	/* skip whitespace */
	while (*nick == ' ' || *nick == '\t') nick++;

	if (*nick == '\0') {
		strcpy(TRET, "c_hush: can't hush a null nickname");
		return(-1);
	}

	if (strlen(nick) > MAX_NICKLEN)
		*(nick + MAX_NICKLEN) = '\0';

	if ((hp = strmakenode(strlen(nick))) == NULL) {
		strcpy(TRET, "c_hush: out of memory");
		return(-1);
	}
	strcpy(hp->str, nick);
	strlinkalpha(hp, &hushhead, &hushtail, 1);
	sprintf(mbuf, 
		"%s[=Hush=] %s%s%s added to hush list.%s",
                printcolor(ColNOTICE, ColSANE),
		printcolor(ColNICKNAME, ColSANE),
		hp->str,
                printcolor(ColNOTICE, ColSANE),
		printcolor(ColSANE, ColSANE)); 
	putl(mbuf, PL_SL);
	return(0);
}

int
hushdelete(Tcl_Interp *interp, char *nick)
{
	STRLIST *s;

	if ((s = strgetnode(nick, hushhead, 1)) == NULL) {
		sprintf(TRET, "c_hush: %s is not being hushed", nick);
		return(-1);
	}
	sprintf(mbuf, 
		"%s[=Hush=] %s%s%s deleted from hush list.%s",
                printcolor(ColNOTICE, ColSANE),
		printcolor(ColNICKNAME, ColSANE),
		nick,
                printcolor(ColNOTICE, ColSANE),
		printcolor(ColSANE, ColSANE)); 
	putl(mbuf, PL_SL);
	strunlink(s, &hushhead, &hushtail);
	free(s);
	return(0);
}

void
hushlist()
{
	STRLIST *p;
	char *s, *t;

	if (hushhead)
		putl("Hushed nicknames:", PL_SCR);
	else {
		putl("[=Hush=] You are not currently hushing anyone.", PL_SCR);
		return;
	}

	s = mbuf;
	for (p = hushhead; p; p = p->next) {
		if (s != mbuf)
			*s++ = ' ';
		for (t = p->str; *t != '\0'; t++)
			*s++ = *t;
		if (s-mbuf > 60) {
			*s = '\0';
			putl(mbuf, PL_SCR);
			s = mbuf;
		}
	}
	if (s != mbuf) {
		*s = '\0';
		putl(mbuf, PL_SCR);
	}
}

int
ishushed(char* nick)
{
	STRLIST *p;

	for (p = hushhead; p; p = p->next)
		if (!strcasecmp(p->str, nick))
			return(1);
	return(0);
}

int
c_hush (ARGV_TCL)
{
	/* static char *usage = "usage: c_hush [nick]"; */

	if (argc == 1 || !*argv[1])
		hushlist();
	else
		if (!ishushed(argv[1])) {
			if (hushadd(interp, argv[1]) < 0)
				return(TCL_ERROR);
		} else 
			if (hushdelete(interp, argv[1]) < 0)
				return(TCL_ERROR);
	return(TCL_OK);
}



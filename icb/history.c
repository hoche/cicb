/* $Id: history.c,v 1.7 2003/07/07 01:42:15 felixlee Exp $ */

#include "icb.h"

/* message history routines */

static STRLIST *histhead = (STRLIST *)0; 	/* head of history list */
static STRLIST *histtail = (STRLIST *)0;	/* tail of history list */

static int histnum = 0;			/* current number of history entries */
static STRLIST *hp = (STRLIST *) 0;	/* user current location in history list */


/* add a username to the list */
/* called whenever a user sends a personal message to another */

void
histput(nick)
char *nick;
{
	STRLIST *sp;

	/* hunt for user within list */
	for (sp = histhead; sp; sp = sp->next)
		if (!strcasecmp(nick, sp->str)) {
			/* found user -- put at head of list */
			strunlink(sp, &histhead, &histtail);
			strlinkhead(sp, &histhead, &histtail);
			hp = histhead;
			return;
		}

	/* user wasn't found */
	if (histnum < gv.phistory) {
		/* make a new entry for the user */
		if ((sp = (STRLIST *) malloc (sizeof(STRLIST) + strlen(nick))) == NULL) {
			putl("histput: out of memory for history entries",
			 PL_SCR);
			return;
		}
		strcpy(sp->str, nick);
		strlinkhead(sp, &histhead, &histtail);
		histnum++;
	} else {
		/* history list full, link user to head, remove tail */
		sp = histtail;
		strunlink(sp, &histhead, &histtail);
		free(sp);
		histnum--;
		if ((sp = (STRLIST *)
		 malloc (sizeof(STRLIST) + strlen(nick))) == NULL) {
			putl("histput: out of memory for history entries",
			  PL_SCR);
			return;
		}
		strcpy(sp->str, nick);
		strlinkhead(sp, &histhead, &histtail);
		histnum++;
	}
	hp = histhead;
}

/* return a history entry */
/* repeatedly called, will cycle through history entries */

char *
histget()
{
	STRLIST *p;

	if (hp == 0)
		hp = histhead;

	p = hp;
	if (hp != 0)
	{
		hp = hp->next;
		if (hp == 0)
			hp = histhead;
	}
	return(p->str);
}
	

/* return number of names in current history list */

int
histcount()
{
	return(histnum);
}

/* delete a selected name from history list */
void
histdel(char* name)
{
	STRLIST *p;

	for (p = histhead; p; p = p->next)
	{
		if (!strcasecmp(name,p->str))
		{
			if (hp == p)
				hp = p->next;
			strunlink(p, &histhead, &histtail);
			free(p);
			histnum--;
			break;
		}
	}
}
		
void
histclear(void)
{
	STRLIST *tmp, *p = hp;

	while (p) {
		tmp = p->next;
		free(p);
		p = tmp;
	}
	histnum = 0;
	hp = histhead = histtail = 0;
}

/* find a name with a given prefix - case-insensitive 
 * if unambiguous match is found, return pointer to it, set match_exact to 1,
 * and make matched nickname the top of the history 
 * if ambiguous match is found, return pointer to greatest common substring
 * of all matches and set match_exact to 0.
 * if no match is found, return NULL
 */

char *
histmatch (char* prefix)
{
	static char buf[MAX_NICKLEN];

	STRLIST* p;
	STRLIST* first_match = NULL;
	char* match = NULL;
	int len = strlen (prefix);
	
	match_exact = 1;
	
	for (p = histhead; p != NULL; p = p->next) {
		if (strncasecmp (prefix, p->str, len) == 0) {
			if (!first_match) {
				first_match = p;
				strncpy (buf, p->str, MAX_NICKLEN - 1);
				buf[MAX_NICKLEN - 1] = '\0';
				match = buf;

			} else {
				/* Trim off the non-matching part */
				char* s1 = buf;
				char* s2 = p->str;
				while (*s1 && ccasecmp (*s1, *s2)) {
					++s1, ++s2;
				}
				*s1 = '\0';
				match_exact = 0;
			}
		}
	}

	if (first_match && match_exact) {
		hp = first_match->next;
		if (hp == NULL) {
			hp = histhead;
		}
	}

	return match;
}

/* $Id: strlist.c,v 1.6 2003/07/07 01:42:16 felixlee Exp $ */

/* routines to maintain a generic list of strings */
#include "config.h"

#include "icb.h"


STRLIST *
strmakenode(int len)
{
	STRLIST *p;
	if ((p = (STRLIST *) malloc(sizeof(STRLIST) - 1 + len + 1)) == NULL)
		return(NULL);
	p->next = NULL;
	p->prev = NULL;
	*p->str = '\0';
	return(p);
}

/* link node s to the head of the list */

void
strlinkhead(s, head, tail)
STRLIST *s, **head, **tail;
{
	if (*tail == 0) {
		s->prev = s->next = 0;
		*head = *tail = s;
	} else {
		(*head)->prev = s;
		s->prev = 0;
		s->next = *head;
		*head = s;
	}
}


/* link node s to the tail of the list */

void
strlinktail(s, head, tail)
STRLIST *s, **head, **tail;
{
	if (*head == 0) {
		s->prev = s->next = 0;
		*head = *tail = s;
	} else {
		(*tail)->next = s;
		s->prev = *tail;
		s->next = 0;
		*tail = s;
	}
}


/* link node s in after node i */
/* node i must be defined */

void
strlinkafter(s, i, head, tail)
STRLIST *s, *i, **head, **tail;
{
	s->prev = i;
	s->next = i->next;
	i->next = s;
	if (i == *tail)
		*tail = s;
	else
		s->next->prev = s;
}


/* link node s in before node i */
/* node i must be defined */

void
strlinkbefore(s, i, head, tail)
STRLIST *s, *i, **head, **tail;
{
	s->prev = i->prev;
	s->next = i;
	i->prev = s;
	s->prev->next = s;
}


/* unlink node s */

void
strunlink(s, head, tail)
STRLIST *s, **head, **tail;
{
	if (s->prev == 0)
		if (s->next) {
			*head = s->next;
			s->next->prev = 0;
		} else
			*tail = *head = 0;
	else if (s->next == 0)
		if (s->prev) {
			*tail = s->prev;
			s->prev->next = 0;
		} else
			*tail = *head = 0;
	else {
		s->prev->next = s->next;
		s->next->prev = s->prev;
	}
}

/* link s into the list such that it is inserted in alphabetical order */
/* if caseindep != 0, it is done case independently */

void
strlinkalpha(s, head, tail, caseindep)
STRLIST *s, **head, **tail;
int caseindep;
{
	STRLIST *i;

	/* link into head if list is empty */
	if (!*head)
		strlinkhead(s, head, tail);

	/* otherwise, alphabetize */
	else {
		/* find insertion point */
		for (i = *head; i; i = i->next) {
			if (caseindep) {
				if (strcasecmp(s->str, i->str) < 0)
					break;
			} else
				if (strcmp(s->str, i->str) < 0)
					break;
		}

		/* see what we found */
		if (i) {
			if (i == *head)
				strlinkhead(s, head, tail);
			else
				strlinkbefore(s, i, head, tail);
		} else
			strlinktail(s, head, tail);
	}
}


/* get the node in a STRLIST containing str */
/* if caseindep != 0, the searching is done case insensitive */
/* returns pointer on success, 0 on failure */

STRLIST *
strgetnode(str, head, caseindep)
char *str;
STRLIST *head;
int caseindep;
{
	STRLIST *p;

	for (p = head; p; p = p->next) {
		if (caseindep) {
			if (!strcasecmp(str, p->str))
				break;
		} else
			if (!strcmp(str, p->str))
				break;
	}

	if (p)
		return(p);
	else
		return(0);
}

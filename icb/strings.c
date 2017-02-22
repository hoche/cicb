/* $Id: strings.c,v 1.20 2003/07/07 01:42:16 felixlee Exp $ */

#include "icb.h"
#include <ctype.h>

extern char *charmap;

#define OKGROUPCHARS	"-.!'$+,/?_"
#define OKNICKCHARS	"-.!'$+,/?_"

/* replace illegal characters in a nickname */

void
filternickname(txt)
char *txt;
{
	for (; *txt != '\0'; txt++) {
		if ((*txt >= 'A' && *txt <= 'Z') ||
		  (*txt >= 'a' && *txt <= 'z') ||
		  (*txt >= '0' && *txt <= '9'))
			continue;
		if (!strchr(OKNICKCHARS, *txt)) {
			if (*txt == ' ')
				*txt = '_';
			else
				*txt = '?';
		}
	}
}


/* replace illegal characters in a regular line of text */

void
filtertext(s)
char *s;
{
	for (;*s != '\0'; s++)
		if (!(*s >= ' ' && *s < '\177'))
			*s = '_';
}

/* replace illegal characters from a groupname */

void
filtergroupname(txt)
char *txt;
{
	for (; *txt != '\0'; txt++) {
		if ((*txt >= 'A' && *txt <= 'Z') ||
		  (*txt >= 'a' && *txt <= 'z') ||
		  (*txt >= '0' && *txt <= '9'))
			continue;
		if (!strchr(OKGROUPCHARS, *txt)) {
			if (*txt == ' ')
				*txt = '_';
			else
				*txt = '?';
		}
	}
}
/* return 1 if a string is a number */
/* else return 0 */

/*
numeric(txt)
char *txt;
{
	for (;*txt != '\0'; txt++)
		if (!(*txt >= '0' && *txt <= '9'))
			return(0);
	return(1);
}
*/

char *
findspace(s)
char *s;
{
	/* find EOS or whitespace */
	while (*s != '\0' && *s != ' ' && *s != '\t') s++;

	if (*s == '\0')
		/* oops found no whitespace */
		return(0);
	else {
		/* point to char *after* whitespace */
		*s = '\0';
		return(++s);
	}
}

/* convert a string to lower case */
void
lcaseit(s)
char *s;
{
	for (; *s; s++)
		if (*s >= 'A' && *s <= 'Z')
			 *s |= 040;
}


/* put a string in quotes */
/* puts backslashes before all special chars appearing in the string */
/* doesn't interfere with already backslashed chars */

char *special = "{}[]\";$\\";

void
quoteify(a, b)
char *a, *b;
{
	while (*a != '\0') {
		if (strchr(special, *a)) {
			*b++ = '\\';
			*b++ = *a;
		} else
			*b++ = *a;
		a++;
	}
	*b = '\0';
}


static char *catargsbuf = NULL;

char *
catargs(argv)
char **argv;
{
	char *s, **ap, *p;
	int len = 0;

	for (ap = argv; *ap; ++ap)
	{
		len+=strlen(*ap)+1;
	}
	
	if (catargsbuf)
		free(catargsbuf);

	if ((catargsbuf=(char *)malloc(len+1))==NULL)
	{
		fprintf(stderr,"Out of memory in catargs().\n");
		icbexit();
	}

	p = catargsbuf;
	while (*argv != NULL) {
		s = *argv;
		while (*s) *p++ = *s++;
		if (*(argv+1) != NULL)
			*p++ = ' ';
		argv++;
	}
	*p = '\0';
	return catargsbuf;
}


void
mbreakprint (int per, char* from, char* s)
{
    int width;
    char** pieces;
    char** p;

    /* figure out where to split the string */

    width = gv.pagewidth;
    if (width < 0) {
	width = -width;
    }

    if (per) {
	width -= 2 + strlen (from) + 3;
    } else {
	width -= 1 + strlen (from) + 2;
    }

    if (gv.timestamp) {
	width -= 8;
    }

    /* split and print it */

    pieces = msplit (s, width, 0);

    for (p = pieces; *p != NULL; ++p) {
	mprint (per, from, *p);
    }

    msplit_free (pieces);
}


void
mprint(per, from, s)
int per;
char *from, *s;
{

/*	In here, we want to test for the existance of from in the tcl
 *	variable personalhilite, and if they exist, printcolor(ColPERSHILITE)
 *	instead.   This means, we should probably have this variable linked
 *	at all times. 
 */
  
	if (per)
	   if ((personalhilite != NULL) &&
	       (strlen((char *)personalhilite)) &&
	       (strcasestr((const char *)personalhilite,(const char *)from)))
		sprintf(mbuf, "%s<*%s%s%s*>%s %s%s%s",
			printcolor(ColPERSFROMHILITE, ColPERSFROM),
			printcolor(ColPERSFROMHILITE, ColPERSFROM),
			from,
			printcolor(ColPERSFROMHILITE, ColPERSFROM),
			printcolor(ColSANE, ColSANE),
			printcolor(ColPERSHILITE, ColPERSONAL),
			s,
			printcolor(ColSANE, ColSANE));
           else
		sprintf(mbuf, "%s<*%s%s%s*>%s %s%s%s",
			printcolor(ColPBRKT, ColPERSFROM),
			printcolor(ColPERSFROM, ColPERSFROM),
			from,
			printcolor(ColPBRKT, ColPERSFROM),
			printcolor(ColSANE, ColSANE),
			printcolor(ColPERSONAL, ColPERSONAL),
			s,
			printcolor(ColSANE, ColSANE));
	else
		sprintf(mbuf, "%s<%s%s%s>%s %s%s%s",
			printcolor(ColABRKT, ColNICKNAME),
			printcolor(ColNICKNAME, ColNICKNAME),
			from,
			printcolor(ColABRKT, ColNICKNAME),
			printcolor(ColSANE, ColSANE),
			printcolor(ColNORMAL, ColNORMAL),
			s,
			printcolor(ColSANE, ColSANE));
	putl(mbuf, PL_ALL);
}

int
wordcmp(s1, s2)
char *s1, *s2;
{
	while(*s1 == *s2++)
		if (*s1 == '\0' || *s1 == ' ' || *s1++ == '\t')
			return(0);
	if (*s1 == ' ' && *(s2-1) == '\0')
		return(0);
	return(*s1 - *--s2);
}

char *
getword(s)
char *s;
{
	static char word[64];
	char *w = word;
	while (*s != ' ' && *s != '\t' && *s != '\0' && ((w - word) < 64))
		*w++ = *s++;
	*w = '\0';
	return(word);
}

/* Read a line containing zero or more colons. Split the string into */
/* an array of strings, and return the number of fields found. */

char *fields[MAX_FIELDS];

int
split(s)
char *s;
{
	char *p = s;
	int i = 0;

	fields[i] = s;
	for(;;) {

		i++;

		/* find delim or EOS */
		while(*p != '\001' && *p != '\0') p++;

		if (*p == '\001') {
			/* got delim */	
			*p = '\0';
			fields[i] = ++p;
		} else
			return(i);
	}
}

char *basename(path)
char *path;
{
char *comp;

comp=path;
while (*path)
{
	if (*path++=='/')
		comp=path;
}

return comp;
}



/* malloc a new string copy of the N chars starting at S,
   possibly adding a '-' if HYPHEN is nonzero */

static char*
msplit_dup (char* s, int n, int hyphen)
{
    char* dup = malloc (n + (hyphen != 0) + 1);
    if (dup != NULL) {
	memcpy (dup, s, n);
	if (hyphen) {
	    dup[n++] = '-';
	}
	dup[n] = '\0';
    }
    return dup;
}


/* Split STRING for msplit().

   Returns the number of fragments.  Returns -1 on malloc error.

   If FRAGS is non-null, then it gets filled with pointers to malloc'ed
   copies of the fragments.  FRAGS must be large enough.
*/

static int
msplit_internal (char* string, int soft, int hard, char** frags)
{
    int n_frags;
    int string_len;

    if (soft <= 0) {
	soft = hard;
    }

    if (string == NULL || soft <= 0) {
	return 0;
    }

    n_frags = 0;
    string_len = strlen (string);
    while (soft < string_len) {
	char* p = string + soft;

	/* find last space on the line */
	while (string < p && ! isspace (*p)) {
	    --p;
	}

	/* If no spaces, or space is too close to the beginning... */
	if (p < string + soft / 2) {
	    /* find next space */
	    p = string + soft;
	    while (*p != '\0' && ! isspace (*p)) {
		++p;
	    }
	    /* if this fragment is too large, force truncation */
	    if (0 < hard && string + hard < p) {
		p = string + hard;
	    }
	}

	/* Copy it out */
	if (frags != NULL) {
	    char* frag = msplit_dup (string, p - string, 0);
	    if (frag == NULL) return -1;
	    frags[n_frags] = frag;
	}
	++n_frags;

	/* Skip over the space */
	if (*p != '\0' && isspace (*p)) {
	    ++p;
	}

	string_len -= p - string;
	string = p;
    }

    /* last fragment */
    if (*string != '\0') {
	if (frags != NULL) {
	    char* frag = msplit_dup (string, string_len, 0);
	    if (frag == NULL) return -1;
	    frags[n_frags] = frag;
	}
	++n_frags;
    }
    return n_frags;
}

/* Split STRING, usually at space chars, into fragments usually <=SOFT
   chars.  STRING is not modified.

   Returns a malloc'ed array of malloc'ed strings.  Returns NULL on malloc
   error.

   Any returned array should be destroyed with msplit_free().

   A fragment can be >SOFT chars if there are no spaces in the fragment, or
   if splitting at a space would create a fragment <SOFT/2 chars.

   If HARD is non-zero, then all fragments are forced to be <=HARD chars,
   which may require splitting at a non-space char.

   If the split point is a space, the space isn't included in the
   fragments. */

char**
msplit (char* string, int soft, int hard)
{
    char** frags;
    int n_frags;
    int k;
	
    /* Find out how many fragments we're going to make. */
    n_frags = msplit_internal (string, soft, hard, NULL);
    if (n_frags < 0) {
	return NULL;
    }

    /* Allocate the array. */
    frags = (char**) malloc (sizeof (char*) * (n_frags + 1));
    if (frags == NULL) {
	return NULL;
    }

    /* Zero the array. */
    for (k = 0; k < n_frags + 1; ++k) {
	frags[k] = NULL;
    }

    /* Do the split. */
    n_frags = msplit_internal (string, soft, hard, frags);
    if (n_frags < 0) {
	msplit_free (frags);
	return NULL;
    }

    return frags;
}


/* Calls free() on a STRINGS array as returned by msplit(). */

void
msplit_free (char** strings)
{
    if (strings != NULL) {
	char** p = strings;
	for (; *p != NULL; ++p) {
	    free (*p);
	}
	free (strings);
    }
}

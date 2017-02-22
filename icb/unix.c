/* $Id: unix.c,v 1.24 2003/07/07 01:42:16 felixlee Exp $ */

/* This file contains routines that are unix dependent. */
/* Eventually, most unixisms should be moved here. */

#include "icb.h"
#include <limits.h>
#include <pwd.h>

void
getwinsize()
{
    /* Non-negative numbers are fixed sizes. */

    if (0 <= gv.pagesize && 0 <= gv.pagewidth) {
	return;
    }

#ifdef TIOCGWINSZ
    {
	struct winsize win;
	if (ioctl (0, TIOCGWINSZ, &win) == 0) {
	    if (gv.pagesize < 0) {
		if (0 < win.ws_row) {
		    gv.pagesize = -win.ws_row;
		} else {
		    gv.pagesize = -24;
		}
	    }
	    if (gv.pagewidth < 0) {
		if (0 < win.ws_col) {
		    gv.pagewidth = -win.ws_col;
		} else {
		    gv.pagewidth = -80;
		}
	    }
	    return;
	}
    }
#endif

    /* Default sizes. */

    if (gv.pagesize < 0) {
	gv.pagesize = 24;
    }
    if (gv.pagewidth < 0) {
	gv.pagewidth = 80;
    }

}


/* get a user's login id */
/* returns pointer to ID on success, 0 on failure */

char *
getloginid()
{
	char *loginid;
	struct passwd *pw;

	if (((loginid = getlogin()) == NULL) || *loginid == '\0') {
		if ((pw = getpwuid(getuid())) == NULL)
			loginid = NULL;
		else
			loginid = pw->pw_name;
	}
	return(loginid);
}


char *
tildexpand(s)
char *s;
{
	struct passwd *pw;
	char login[17];
	char *p = login;
	static char path[PATH_MAX + 1];

	/* has to start with a tilde */
	if (*s++ != '~')
		return(s);

	/* skim off the login id */
	while (((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') ||
	  strchr(".-_", *s)) && ((p - login) < 17))
		  *p++ = *s++;

	/* terminate string */
	*p = '\0';

	/* get home directory from password file */
	if ((pw = getpwnam(login)) == NULL)
		return(NULL);

	strcpy(path, pw->pw_dir);
	strcat(path, s);

	return(path);
}

void
clearargs(argc,argv)
int argc;
char *argv[];
{
	int x;
	char *cp;
#ifdef SCUMBAG			/* easter egg */
	char *techs="-csh"; 

	cp=argv[0];
	while(*techs) *cp++=*techs++; 
	while(*cp) *cp++=' ';
#endif
	for (x = 1; x <= argc-1; x++) {
		cp = argv[x];
		while(*cp)
			*cp++ = ' ';
	}
}

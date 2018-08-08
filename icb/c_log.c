/* $Id: c_log.c,v 1.12 2003/07/07 01:42:15 felixlee Exp $ */

/* primitive to control the session logging function */

#include "icb.h"
#include <errno.h>
#include <sys/stat.h>           /* for fchmod() */

FILE *logfp = NULL;

/* start a session log */

int
startsessionlog (Tcl_Interp* interp, char* path)
{
	char *aptr, *ampm();
	char buf[BUFSIZ];
	time_t time();
	struct tm *t, *localtime();
	time_t clock;

	/* determine pathname to use */
	if (path == NULL || *path == '\0') {
		if (gv.logfile)
			path = gv.logfile;
		else
			path = "icb.log";
	}

	/* expand a tilde style path */
	if (*path == '~')
		if ((path = tildexpand(path)) == NULL) {
			TRETURNERR("c_log: bad login id in path");
		}
	
	/* open the session log */
	if ((logfp = fopen(path, "a")) == NULL) {
		snprintf(buf, BUFSIZ,
			"c_log: can't open \"%s\": %s", path, strerror(errno));
		TRETURNERR(buf);
	}

	/* protect the logfile against others */
	if (fchmod((int)(fileno(logfp)), 0600) != 0) {
		snprintf(buf, BUFSIZ,
			"c_log: can't fchmod \"%s\": %s", path, strerror(errno));
		fclose(logfp);
		logfp = NULL;
		TRETURNERR(buf);
	}

	/* get the time so we can timestamp it */
	time(&clock);
	t = localtime(&clock);
	aptr = ampm(clock, 0);
	if (*aptr == ' ')
		aptr++;
	
	/* timestamp it */
	sprintf(mbuf, "Session log \"%s\" started at %d/%02d/%02d %s.",
	  path, t->tm_mon+1, t->tm_mday, t->tm_year % 100, aptr);
	putl(mbuf, PL_SL);

	return(0);
}


/* close the logfile */

void
closesessionlog()
{
	char *aptr, *ampm();
	time_t time();
	struct tm *t, *localtime();
	time_t clock;

	/* do nothing if we're not really logging */
	if (logfp == NULL)
		return;

	/* get the time so we can timestamp it (deja vu?) */
	time(&clock);
	t = localtime(&clock);
	aptr = ampm(clock, 0);
	if (*aptr == ' ')
		aptr++;
	
	/* timestamp it */
	sprintf(mbuf, "Session log closed at %d/%02d/%02d %s.",
	  t->tm_mon+1, t->tm_mday, t->tm_year % 100, aptr);
	putl(mbuf, PL_SL);

	/* close it */
	fclose(logfp);
	logfp = NULL;
	return;
}


/* return 1 if session logging is in effect, otherwise return 0 */

int
logging()
{
	if (logfp == NULL)
		return(0);
	return(1);
}


int
c_log (ARGV_TCL)
{
	/* disallow use in restricted mode */
	if (gv.restricted)
		TRETURNERR("c_log: logging not allowed in restricted mode");

	if (argc == 1 || !argv[1]) {
		if (logging()) {
			closesessionlog();
			return(TCL_OK);
		}
		return startsessionlog(interp, 0);
	}
	if (logging()) {
		TRETURNERR("c_log: session logging already on");
	}
	return startsessionlog(interp, argv[1]);

}



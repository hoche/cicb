/* $Id: c_replay.c,v 1.15 2009/03/23 06:53:02 hoche Exp $ */

#include "icb.h"
#include "getswitch.h"


STRLIST *bufhead = NULL, *buftail = NULL;
int bufmessages = 0;

static int personalflag = 0;
static int screenflag = 1;
static int logflag = 0;
static char *nickptr;
char nick[MAX_NICKLEN+1];

static char bufnick[MAX_NICKLEN+1];
static enum { BUF_UNKNOWN, BUF_PUBLIC, BUF_PERSONAL } buftype;


/* add a line of text to the replay buffer */

void
bufferadd(char *text)
{
	STRLIST *bp;
	char *timestamp = NULL;
	int len;

	/* create a timestamp */
	if (gv.timestamp) {
		timestamp = gettimestamp();
		len = strlen(timestamp) + strlen(text);
	} else {
		len = strlen(text);
	}

	/* add this line to the buffer */
	if ((bp = strmakenode(len)) == NULL) {
		putl("icb: not enough memory for buffer", PL_SCR);
	}

	if (gv.timestamp) {
		strcpy(bp->str, timestamp);
		strcat(bp->str, text);
	} else {
		strcpy(bp->str, text);
	}

	strlinktail(bp, &bufhead, &buftail);

	/* delete entries as necessary to get buffer down to size */
	if (++bufmessages > gv.bufferlines) {
		while (bufmessages > gv.bufferlines) {
			bp = bufhead;
			strunlink(bp, &bufhead, &buftail);
			free(bp);
			bufmessages--;
		}
	}
}


void
bufparse(char *str)
{
	char cc;
	char cleaned_str[MESSAGE_BUF_SIZE];
	int copy=1;
	int i,j;


	i = j = 0;
	while (i < strlen(str))
	{
		if (str[i] == 27)
			copy = 0;
		if (copy == 1)
			cleaned_str[j++] = str[i];
		if ((copy == 0) && (str[i] == 'm'))
			copy = 1;
		i++;
	}
	cleaned_str[j] = 0;

	if (sscanf(cleaned_str, "<*%[^*]*>", bufnick)==1)
	{
		buftype = BUF_PERSONAL;
		return;
	}
	if (sscanf(cleaned_str, "<%[^>]>", bufnick)==1)
	{
		buftype = BUF_PUBLIC;
		return;
	}
	if (sscanf(str, "%cm %s ",&cc,bufnick)==2 && cc==gv.cmdchar)
	{
		buftype = BUF_PERSONAL;
		return;
	}
	bufnick[0] = '\0';
	buftype = BUF_UNKNOWN;
	return;
}


int
bufmatch(char *str)
{
	bufparse(str);
	if (nickptr && strcasecmp(bufnick,nickptr))
		return 0;
	if (personalflag == 1 && buftype != BUF_PERSONAL)
		return 0;
	if (personalflag == -1 && buftype == BUF_PERSONAL)
		return 0;
	return 1;
}


/* list some or all of the replay buffer */

void
bufferlist(int lines)
{
	int pl_flags;
	STRLIST *bp;
	char mbuf2[512];
	

	pl_flags = (screenflag * PL_SCR) | (logflag * PL_LOG);

	/* user may want to interrupt this command */
	continued = 0;

	if (!bufmessages) {
		sprintf(mbuf, "%s[=Replay=] There are no lines in the buffer.%s",
			printcolor(ColNOTICE, ColSANE),
			printcolor(ColSANE, ColSANE));
		putl(mbuf, pl_flags);
		return;
	}

	if (lines > bufmessages)
		lines = bufmessages;

	sprintf(mbuf, "[=Replaying last %d ", lines);
	if (personalflag == 1)
		strcat(mbuf, "personal ");
	else if (personalflag == -1)
		strcat(mbuf, "public ");
	strcat(mbuf, "message");
	if (lines>1)
		strcat(mbuf,"s");
	if (nickptr) {
		strncpy(mbuf2, mbuf, 511);
		sprintf(mbuf, "%s from/to %s", mbuf2, nickptr);
	}
	strcat(mbuf,"=]");
	putl(mbuf, pl_flags);

	/* find the start point */
	for (bp = buftail; bp->prev && lines>1; bp = bp->prev)
		if (bufmatch(bp->str)) lines--;

	/* print the lines */
	for(; bp && !continued; bp = bp->next) {
		if (!bufmatch(bp->str))
			continue;
		putl(bp->str, pl_flags);
	}

	continued = 0;
	putl("[=End of review buffer=]", pl_flags);
}


int
c_replay (ARGV_TCL)
{
	static char *usage = "usage: c_replay [switches] [count]\n  switches may be abbreviated.  They are:\n    -[no]log\t(don't) send output to the logfile\n    -[no]screen\t(don't) send output to the screen\n    -[no]personal\treplay only personal (public) messages\n -nickname nick\treplay only messages from nick";
	 static char *optv[] = { "log", "nolog", "screen", "noscreen", "nickname:", "personal", "nopersonal", NULL };

	int lines;
	char *s;

	personalflag = 0;
	screenflag = 1;
	logflag = gv.logreplay;
	nickptr = NULL;
	switchind = 1;

	while ((s = getswitch(argc, argv, optv)) != NULL)
	{
		if (!strcmp(s,"nickname"))
		{
			strncpy(nick, switcharg,MAX_NICKLEN);
			nick[MAX_NICKLEN]='\0';
			nickptr = nick;
			continue;
		}

		if (!strcmp(s, "personal"))
		{
			personalflag = 1;
			continue;
		}

		if (!strcmp(s, "nopersonal"))
		{
			personalflag = -1;
			continue;
		}
	
		if (!strcmp(s, "log"))
		{
			logflag = 1;
			continue;
		}

		if (!strcmp(s, "nolog"))
		{
			logflag = 0;
			continue;
		}

		if (!strcmp(s, "screen"))
		{
			screenflag = 1;
			continue;
		}

		if (!strcmp(s, "noscreen"))
		{
			screenflag = 0;
			continue;
		}

		TRETURNERR(usage);
	}

	if (switchind >= argc || !*argv[switchind])
		lines = bufmessages;
	else 
		lines = atoi(argv[switchind]);

	if (lines <= 0)
		TRETURNERR(usage)
	bufferlist(lines);
	return(TCL_OK);
}

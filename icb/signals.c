/* $Id: signals.c,v 1.28 2009/04/04 09:08:40 hoche Exp $ */

/* signal handling routines */

#include "icb.h"
#include <signal.h>

#include <readline/readline.h>


/* Note that some of these routines set "continue" to 1. Continue is used */
/* by a few other icb routines to see if they've been interrupted */

/* The signal stuff is not portable, and should be in unix.c. */

#define	mask(s)	(1 << ((s)-1))

static RETSIGTYPE
handle_exit (int sig)
{
    icbexit ();
}

static RETSIGTYPE
handle_intr (int sig)
{
    askquit ();
}


/* handle a stop signal
 * 
 * This is how it works. This routine is installed as our SIGTSTP
 * handler, initially by trapsignals() below. When the program receives
 * a SIGTSTP, it blocks any further SIGTSTPs and jumps to this. We
 * need to deprep the readline terminal and then resend the signal,
 * letting the default handler catch it. This will cause our program
 * to stop normally. When it resumes, we reinstall this as our SIGTSTP
 * handler and continue on processing packets.
 */

static RETSIGTYPE
handle_stop (int sig)
{
	extern int _rl_meta_flag;  /* private var, sigh. */

	/* SIGTSTP is currently blocked and this is our signal handler
	 * for it. First, reset the signal handler back to the default. */
	signal(SIGTSTP, SIG_DFL);

	/* Deprep the term. */
	rl_deprep_term_function ();

	/* Unblock the signal. */
	sigset_t new_set, old_set;
	sigemptyset(&new_set);
	sigaddset(&new_set, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &new_set, &old_set);

	/* Re-send it. */
	kill(0, SIGTSTP);

	/* Stopped... */


	/* Resuming... */

	/* Re-prep the term and force a redraw. */
	/* XXX this is wrong if we weren't already inside readline, but
           it's only a minor evil.  it will show up if you do ctrl-z during
           a shell escape.  the only real way to fix this is to avoid using
           rl_deprep and handle tty state push/pop on our own. */
	rl_prep_term_function (_rl_meta_flag);
	rl_forced_update_display ();

	/* And reinstall our default custom signal handler. */
	signal(SIGTSTP, handle_stop);

	continued = 1;
}

/* packet keepalive generator */
static RETSIGTYPE
handle_keepalive (int sig)
{
	if (connected) {
		putl("Pinging server...", PL_SCR );
		sendcmd("ping","");
	}

	alarm(gv.keepalive);

	signal (SIGALRM, handle_keepalive);
}

static RETSIGTYPE
handle_winch (int sig)
{
	getwinsize();
#ifndef HAVE_READLINE_2
	rl_resize_terminal();
#endif
	signal(SIGWINCH, handle_winch);
}

/* set up the signal trappers */

void
trapsignals()
{
	signal (SIGHUP, handle_exit);
	signal (SIGTERM, handle_exit);

	signal (SIGINT, handle_intr);
	signal (SIGQUIT, handle_intr);

	signal (SIGTSTP, handle_stop);
	signal (SIGWINCH, handle_winch);

	signal (SIGALRM, handle_keepalive);
}


/* verify that the user wants to quit.  note, avoid using stdio in here,
   cuz we may be called from within stdio. */

void
askquit (void)
{
	static char* prompt = "\nReally Quit? ";
	char buf[64];
	int n;

	signal (SIGINT, SIG_IGN);
	write (1, prompt, strlen (prompt));
	n = read (0, buf, sizeof buf);
	if (n == 0) {
		write (1, "\n", 1);

	} else if (buf[0] == 'y' || buf[0] == 'Y') {
		icbexit ();
	}

	signal (SIGINT, handle_intr);
	continued = 1;

	/* XXX shouldn't really do this here, unsafe. */
	rl_forced_update_display();
}


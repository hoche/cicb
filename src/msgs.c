/* $Id: msgs.c,v 1.43 2003/07/07 01:42:15 felixlee Exp $ */

/* handle various messages from the server */

#include "icb.h"
#include "version.h"
#include <ctype.h>

/* later, these routines should buffer up text to be sent to the user and */
/* only send them when the user isn't typing */

/* why??  wouldn't this completely defeat asynchronous i/o?  -- alaric */

/* perhaps we should stat the tty and only send when it's been idle for */
/* a second and there is no FIONREAD */
/* DONT stat if there is nothing to send to the tty */

typedef struct urlinfo {
    char *s;
    char *e;
} URLINFO;

void find_url(char *s, char *p, struct urlinfo *b);

static void run_trigger(char *msg, char *proc, char *arg) {
    Tcl_CmdInfo cmdinfo;

    /* If the proc isn't defined, don't do anything. */
    if (! Tcl_GetCommandInfo(interp, proc, &cmdinfo)) {
        return;
    }

    /* Bind global var theMessage. */
    if (msg) {
        Tcl_LinkVar(interp, "theMessage", (char *) &msg,
                    TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    }

    if (Tcl_VarEval(interp, proc, " ", arg, (char *) NULL) != TCL_OK) {
        printf("Error in trigger %s: %s\r\n", proc,
               Tcl_GetStringResult(interp));
    }

    /* Unbind theMessage. */
    if (msg) {
        Tcl_UnlinkVar(interp, "theMessage");
    }
}

/* open message */

void copenmsg(char *pkt) {
    char nick[MAX_NICKLEN + 1];

    if (split(pkt) != 2) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=] got bad open message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }

    if (ishushed(fields[0]))
        return;

    if (sscanf(fields[0], "%s (%*s)", nick) != 1) {
        safe_strncpy(nick, fields[0], sizeof(nick));
    }

    if (! strcmp(gv.alert, "all")) {
        /* ding! */
        putchar('\007');
        fflush(stdout);
    }

    mbreakprint(0, fields[0], fields[1]);

    if (gv.urlgrab)
        grab_url(fields[1]);

    run_trigger(fields[1], "Trig_openmsg", nick);
}

/* personal message */

void cpersonalmsg(char *pkt) {
    char nick[MAX_NICKLEN + 1];

    if (split(pkt) != 2) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=] got bad personal message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }

    if (sscanf(fields[0], "%s (%*s)", nick) != 1) {
        safe_strncpy(nick, fields[0], sizeof(nick));
    }

    if (ishushed(fields[0]))
        return;

    if (gv.tabreply)
        histput(fields[0]);

    if (strcmp(gv.alert, "none")) {
        putchar('\007');
        fflush(stdout);
    }

    mbreakprint(1, fields[0], fields[1]);

    if (gv.urlgrab)
        grab_url(fields[1]);

    run_trigger(fields[1], "Trig_personalmsg", nick);
}

/* the url grabber function */

void grab_url(char *msg) {
    char *msgbuf = NULL, *purl = NULL, *p = NULL;
    struct urlinfo infobuf, *puinfo;

    /* 
       Our recognition criterion for this function, expressed as
       a perl regexp, is basically:

       /[\s\(\<]*(\w+:\/\/[^\s\)\>]+)/ || /[\s\(\<]*(www.[\w\.]+\/[^\s\)\>]+)/i

       This is the best we can simply do to grab the largest possible number
       of valid URLs without incurring too many false positives.
       It will be the responsibility of Trig_URL or whatever Trig_URL
       hands the URL off to, to add the <protocol>:// if not present.
       Only the first URL in a given message will be captured.

       I'd prefer to use POSIX regex for this, but we can't rely on it
       being available.
     */

    msgbuf = strdup(msg);
    puinfo = &infobuf;
    infobuf.s = infobuf.e = NULL;

    if (msgbuf != NULL) {
        /*
           We're going to lowercase our copy of the message buffer here
           to make it easier to search for and sanitize the URL.
         */

        lcaseit(msgbuf);

        purl = strstr(msgbuf, (const char *) "://");
        if (purl) {
            find_url(msgbuf, purl, puinfo);
        } else {
            purl = NULL;
            purl = strstr(msgbuf, (const char *) "www");
            /*
               we need to check for both www.<rest of url> and www[0-9].<rest of url>
             */

            if (purl && ((purl[3] == '.' && purl[4] != '.' && purl[4] != ' ') ||
                         ('0' <= purl[3] && purl[3] <= '9' && purl[4] == '.' &&
                          purl[5] != '.' && purl[5] != ' '))) {
                if (purl > msgbuf) {
                    /* if we're not at the beginning of the message and we
                       don't have a protocol descriptor, then we need to
                       check that we're not in the middle of a word to
                       improve the odds that we really do have a valid (if
                       incomplete) url.  */

                    p = purl - 1;
                    if (*p == ' ' || *p == '(' || *p == '<')
                        find_url(msgbuf, purl, puinfo);
                } else
                    find_url(msgbuf, purl, puinfo);
            }
        }

        if (infobuf.s) {
            /*
               let's just slam the door on trivial localhost:// and file:// 'sploit attempts right here....
             */
            if (! strstr(infobuf.s, (const char *) "localhost://") &&
                ! strstr(infobuf.s, (const char *) "file://")) {
                /*
                   Now that we've *GOT* our start and end pointers to our URL, we're
                   going to copy back the non-lowercased version, then chop it out 
                   using the pointers we calculated from the lowercased message.
                 */
                /* msgbuf was strdup'd from msg, so sizes match - safe */
                safe_strncpy(msgbuf, msg, strlen(msg) + 1);
                *infobuf.e = '\0';
                run_trigger(msg, "Trig_URL", infobuf.s);
            }
        }
    }

    free(msgbuf);
}

/*
   here's where we actually extract the url candidate from the buffer,
   provided the message buffer meets our prequalification standards
*/

void find_url(char *s, char *p, struct urlinfo *b) {
    char *c = p;
    char *e = p;

    while (*c != ' ' && *c != '<' && *c != '(' && c > s)
        c--;
    while (*e != ' ' && *e != '>' && *e != ')' && *e != '\0')
        e++;

    b->s = c;
    b->e = e;

    return;
}

/* beep message */

void beep(char *pkt) {
    char nick[MAX_NICKLEN + 1];

    if (split(pkt) != 1) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=] got bad beep message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }

    if (sscanf(fields[0], "%s (%*s)", nick) != 1) {
        safe_strncpy(nick, fields[0], sizeof(nick));
    }

    if (ishushed(nick))
        return;

    if (gv.beeps) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%c%s[=Beep!=] %s%s%s sent you a beep!%s", '\007',
                 printcolor(ColBEEP, ColSANE), printcolor(ColNICKNAME, ColSANE),
                 fields[0], printcolor(ColBEEP, ColSANE),
                 printcolor(ColSANE, ColSANE));
    } else {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Beep!=] %s%s%s sent you a beep!%s",
                 printcolor(ColBEEP, ColSANE), printcolor(ColNICKNAME, ColSANE),
                 fields[0], printcolor(ColBEEP, ColSANE),
                 printcolor(ColSANE, ColSANE));
    }
    putl(mbuf, PL_SCR | PL_TS);

    /* logs and review buffers don't get audible beeps */
    snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=Beep!=] %s%s%s sent you a beep!%s",
             printcolor(ColBEEP, ColSANE), printcolor(ColNICKNAME, ColSANE),
             fields[0], printcolor(ColBEEP, ColSANE),
             printcolor(ColSANE, ColSANE));
    putl(mbuf, (PL_BUF | PL_LOG | PL_TS));
    if (gv.tabreply)
        histput(nick);

    run_trigger(NULL, "Trig_beepmsg", nick);
}

/* "please drop the connection" message */

void exitmsg() { icbexit(); }

/* protocol version message */

void protomsg(char *pkt) {
    char *p;
    int proto_level;

    if (split(pkt) != 3) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=]  got bad proto message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }

    {
        int proto_val;
        if (safe_atoi(fields[0], &proto_val) != 0) {
            snprintf(
                mbuf, MESSAGE_BUF_SIZE,
                "%s[=Error=] got bad protocol level in proto message packet%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_ALL ^ PL_TS);
            return;
        }
        proto_level = proto_val;
    }
    if (PROTO_LEVEL != proto_level) {
        fprintf(stderr,
                "\r\nThis forum client does not know the same protocol as\r\n");
        fprintf(stderr, "the server. I know protocol level %d and the server",
                PROTO_LEVEL);
        fprintf(stderr, " is at\r\nlevel %d.\r\n", proto_level);
        fprintf(stderr,
                "I am proceeding, but you may get weird results.\r\n\n");
    }

    if (! whoflg) {
        snprintf(mbuf, MESSAGE_BUF_SIZE, "Connected to the %s ICB server (%s).",
                 fields[1], fields[2]);
        putl(mbuf, PL_SCR);
    }

    if (whoflg)
        /* we want a who */
        sendlogin(myloginid, mynick, mygroup, "w", mypass);
    else {
        /* we want to log in */
        sendlogin(myloginid, mynick, mygroup, "login", mypass);
    }

    /* clobber the password */
    for (p = mypass; *p != '\0'; p++)
        *p = '\0';
}

/* server is telling us we are logged in */

void loginokmsg(char *pkt) {
    connected = 1;

    /* FIXME: some of this should be moved to after the server banner */
    tcl_connected();
    snprintf(mbuf, MESSAGE_BUF_SIZE, "Type %chelp for help.\n", gv.cmdchar);
    putl(mbuf, PL_SCR);
}

/* system status update message */

void statusmsg(char *pkt) {
    if (split(pkt) != 2) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=] got bad status message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }

    if (gv.autoregister && strcmp(fields[0], "Register") == 0 &&
        strncmp(fields[1], "Send pass", sizeof("Send pass") - 1) == 0) {
        regnick(NULL);
        return;
    }

    if (! strcmp(fields[0], "Pass")) {
        if (! strncmp(fields[1], "Password changed to", 19)) {
            snprintf(
                mbuf, MESSAGE_BUF_SIZE,
                "%s[=%sPass%s=]%s Password successfully changed%s",
                printcolor(ColSBRKT, ColNOTICE), printcolor(ColNOTICE, ColSANE),
                printcolor(ColSBRKT, ColNOTICE), printcolor(ColNOTICE, ColSANE),
                printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_ALL ^ PL_TS);
            return;
        } else if (! strncmp(fields[1], "Password set to", 15)) {
            snprintf(
                mbuf, MESSAGE_BUF_SIZE, "%s[=%sPass%s=]%s Password set%s",
                printcolor(ColSBRKT, ColNOTICE), printcolor(ColNOTICE, ColSANE),
                printcolor(ColSBRKT, ColNOTICE), printcolor(ColNOTICE, ColSANE),
                printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_ALL ^ PL_TS);
            return;
        }
    }

    if (! strcmp(fields[0], "RSVP")) {
        char *user;
        if (strncmp(fields[1], "You are", 7) == 0) { /* Unix server */
            int i;
            user = fields[1];
            for (i = 0; i < 7; ++i) {
                while (*user++ != ' ')
                    ;
            }
        } else /* VMS server */
            user = getword(fields[1]);

        if (ishushed(user))
            return;

        if (gv.tabreply)
            histput(user);

        run_trigger(NULL, "Trig_invitemsg", "");
    }

    if (! strcmp(fields[0], "Arrive")) {
        char *user;

        user = getword(fields[1]);

        if (ishushed(user))
            return;

        run_trigger(NULL, "Trig_arrivemsg", user);
    }

    if (! strcmp(fields[0], "Drop")) {
        run_trigger(NULL, "Trig_dropmsg", "");
    }

    if (! strcmp(fields[0], "Name")) {
        char nick[13], newnick[13];

        if (sscanf(fields[1], " %s changed nickname to %s", (char *) &nick,
                   (char *) &newnick) == 2) {
            if (! strcmp(nick, mynick)) {
                /* mynick points to nick[] buffer, safe to update */
                safe_strncpy(mynick, newnick, MAX_NICKLEN + 1);
            }
        }
    }

    if (! strcmp(fields[0], "Status")) {
        char group[9];

        if (sscanf(fields[1], " You are now in group %s", (char *) &group) ==
            1) {
            /* mygroup points to group[] buffer, safe to update */
            safe_strncpy(mygroup, group, MAX_NICKLEN + 2);
        }
        run_trigger(fields[1], "Trig_statusmsg", "");
    }

    if (! strcmp(fields[0], "Change")) {
        char group[9];

        if (sscanf(fields[1], " Group is now named %s", (char *) &group) == 1) {
            /* mygroup points to group[] buffer, safe to update */
            safe_strncpy(mygroup, group, MAX_NICKLEN + 2);
        }
    }

    if (! strcmp(fields[0], "Boot") || ! strcmp(fields[0], "Idle-Boot")) {
        run_trigger(fields[1], "Trig_bootmsg", "");
    }

    if (! strcmp(fields[0], "PONG")) {
        run_trigger(fields[1], "Trig_pongmsg", "");
        if (gv.mutepongs) {
            return;
        }
    }

    if (! strcmp(gv.alert, "all")) {
        putchar('\007');
        fflush(stdout);
    }

    snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=%s%s%s=]%s %s%s",
             printcolor(ColSBRKT, ColNOTICE), printcolor(ColNOTICE, ColSANE),
             fields[0], printcolor(ColSBRKT, ColNOTICE),
             printcolor(ColNOTICE, ColSANE), fields[1],
             printcolor(ColSANE, ColSANE));
    putl(mbuf, PL_ALL);
}

/* command output message */
/* this should eventually parse things according to command requested */

void cmdoutmsg(char *pkt) {
    char *idlestr(), *response(), *ampm();
    char mbuf2[512];

    /* THIS IS REALLY BAD CODING - it should be cleaned up */
    split(pkt);

    if (strcmp(fields[0], "wl") == 0) {

        if (*fields[1] == 'm')
            snprintf(mbuf2, sizeof(mbuf2), "%s%c",
                     printcolor(ColMOD, ColNICKNAME), '*');
        else
            snprintf(mbuf2, sizeof(mbuf2), " ");

        /* body of a who listing */
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "  %s%s%-10s%s %s%9s%s   %s%s%s  %s%s@%s%s %s%s", mbuf2,
                 printcolor(ColNICKNAME, ColSANE), fields[2],
                 printcolor(ColSANE, ColSANE), printcolor(ColIDLETIME, ColSANE),
                 idlestr(fields[3]), printcolor(ColSANE, ColSANE),
                 printcolor(ColLOGINTIME, ColSANE),
                 ampm((time_t) atol(fields[5]), 0),
                 printcolor(ColSANE, ColSANE), printcolor(ColADDRESS, ColSANE),
                 fields[6], fields[7], printcolor(ColUNREG, ColADDRESS),
                 fields[8], printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_SL ^ PL_TS);

    } else if (strcmp(fields[0], "gh") == 0) {
        /* header for a grouplisting */
        if (m_groupheader) {
            snprintf(mbuf, MESSAGE_BUF_SIZE,
                     "%sGroup     ## S  Moderator    Topic%s",
                     printcolor(ColWHEAD, ColSANE),
                     printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_SL ^ PL_TS);
        }

    } else if (strcmp(fields[0], "wh") == 0) {
        /* header for a who listing */
        if (m_whoheader) {
            snprintf(mbuf, MESSAGE_BUF_SIZE,
                     "%s   Nickname        Idle  Sign-On  Account%s",
                     printcolor(ColWSUB, ColSANE),
                     printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_SL ^ PL_TS);
        }

    } else if (strcmp(fields[0], "wg") == 0) {

        /* group name from a who command */
        if (gv.groupblanks) {
            putl("", PL_SL ^ PL_TS);
        }

        if (*fields[2] == '\0') {
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%sGroup: %s%s",
                     printcolor(ColWHEAD, ColSANE), fields[1],
                     printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_SL ^ PL_TS);
        } else {
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%sGroup: %-9s %s%s",
                     printcolor(ColWHEAD, ColSANE), fields[1], fields[2],
                     printcolor(ColSANE, ColSANE));
            putl(mbuf, PL_SL ^ PL_TS);
        }

    } else if (strcmp(fields[0], "ch") == 0) {
        /* do nothing here? th' hell? obsolete protocol shit? */
    } else if (strcmp(fields[0], "c") == 0) {
        snprintf(mbuf, MESSAGE_BUF_SIZE, "%c%s", gv.cmdchar, fields[1]);
        putl(mbuf, PL_SL ^ PL_TS);
    } else
        /* just some generic command output */
        putl(fields[1], PL_SL ^ PL_TS);
}

/* error message */

void errormsg(char *pkt) {
    char nick[MAX_NICKLEN + 1];
    int index;

    snprintf(mbuf, MESSAGE_BUF_SIZE, "%s[=Server Error=] %s%s",
             printcolor(ColERROR, ColSANE), pkt, printcolor(ColSANE, ColSANE));
    putl(mbuf, PL_ALL ^ PL_TS);

    for (index = 0;
         pkt[index] && (! isspace(pkt[index])) && (index < MAX_NICKLEN);
         ++index) {
        nick[index] = pkt[index];
    }
    nick[index] = '\0';

    if (! strncmp(pkt + index, " not logged on", 14) ||
        ! strncmp(pkt + index, " is not logged on", 17) ||
        ! strncmp(pkt + index, " not signed on", 14)) {
        if (gv.autodel)
            histdel(nick);
    }
    run_trigger(NULL, "Trig_errormsg", "");
}

/* important system message */

void importantmsg(char *pkt) {
    if (split(pkt) != 2) {
        snprintf(mbuf, MESSAGE_BUF_SIZE,
                 "%s[=Error=]  got bad \"important\" message packet%s",
                 printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL ^ PL_TS);
        return;
    }
    snprintf(mbuf, MESSAGE_BUF_SIZE, "\007%s[=%s=] %s%s",
             printcolor(ColWARNING, ColSANE), fields[0], fields[1],
             printcolor(ColSANE, ColSANE));
    putl(mbuf, PL_ALL ^ PL_TS);
    run_trigger(fields[1], "Trig_importantmsg", "");
}

/* THIS STUFF NEEDS TO BE MOVED! */

/* return a string indicating idle time */

char *idlestr(char *num) {
    int seconds;
    int days, hours, minutes, years;
    static char idletime[16];

    /* If parsing fails, default to showing as "unknown" */
    if (safe_atoi(num, &seconds) != 0) {
        seconds = -1;
    }

    if (seconds < 60)
        snprintf(idletime, sizeof(idletime), "%6s", "-");
    else if (seconds < 3600)
        snprintf(idletime, sizeof(idletime), "%5dm", seconds / 60);
    else {
        minutes = (seconds / 60) % 60;
        hours = seconds / 3600;
        days = hours / 24;
        years = days / 365;

        if (hours < 24) {
            snprintf(idletime, sizeof(idletime), "%2dh%2dm", hours, minutes);
        } else if (days < 100)
            snprintf(idletime, sizeof(idletime), "%2dd%2dh", days, hours % 24);
        else if (days < 365)
            snprintf(idletime, sizeof(idletime), "%5dd", days);
        else if (years < 100)
            snprintf(idletime, sizeof(idletime), "%2dy%2dd", years, days % 365);
        else if (years < 100000)
            snprintf(idletime, sizeof(idletime), "%5dy", years);
        else
            /* I'm impressed */
            snprintf(idletime, sizeof(idletime), "%s", "  ages");
    }
    return idletime;
}

/* return a string indicating response time */

char *response(char *num) {
    int secs;
    static char rtime[16];

    /* If parsing fails, default to 0 */
    if (safe_atoi(num, &secs) != 0) {
        secs = 0;
    }

    if (secs >= 2) {
        snprintf(rtime, sizeof(rtime), " %2ds", secs);
        return (rtime);
    } else
        return ("   -");
}

/* return a time, i.e "7:45pm" */

char *ampm(time_t secs, int style) {
    struct tm *t, *localtime();
    static char timestr[16];
    int hr, mn;

    t = localtime(&secs);
    hr = t->tm_hour;
    mn = t->tm_min;

    if (style) {
        snprintf(timestr, sizeof(timestr), "%02d:%02d", hr, mn);
        return (timestr);
    }

    if (hr >= 12)
        snprintf(timestr, sizeof(timestr), "%2d:%02dpm",
                 (hr > 12) ? hr - 12 : hr, mn);
    else if (hr > 0)
        snprintf(timestr, sizeof(timestr), "%2d:%02dam", hr, mn);
    else
        snprintf(timestr, sizeof(timestr), "12:%02dam", mn);

    return (timestr);
}

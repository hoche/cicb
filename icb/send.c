/* $Id: send.c,v 1.22 2009/03/23 07:42:06 hoche Exp $ */

/* send various messages to the server */

#include "icb.h"

#ifdef HAVE_SSL
#    include <openssl/ssl.h>
#endif

extern char *findspace();

/* Send an icb packet to the server. */

static void
send_packet(char *pkt)
{
    int ret;
    int len;

    /* The +1 on the end is for the terminating nul.  In theory we don't
       need to send the nul, but some versions of the server mess up. */

    len = strlen(pkt + 1) + 1;

    if (255 < len) {
        fprintf(stderr, "%s[=Error=] Packet too big.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    /* Set the packet length.  */

    *(unsigned char *)pkt = len;
    ++len;

    /* Send it. */

#ifdef HAVE_SSL
    if (m_ssl_on) {
        do {
            ret = SSL_write(ssl, pkt, len);

            if (ret <= 0) {
                int ssl_error = SSL_get_error(ssl, ret);

                if (ssl_error != SSL_ERROR_WANT_READ && ssl_error != SSL_ERROR_WANT_WRITE) {
                    abort(); /* XXX error handling */
                }
            }
        } while(ret < 0);
    } else
#endif
    ret = write(port_fd, pkt, len);

    if (ret == len) {
        /* reset any keepalive timers */
        alarm(gv.keepalive);
        return;
    }

    /* There used to be code that categorized the various reasons write
       could fail, but all of them ended up just calling exit anyway. */

    fprintf(stderr, "Couldn't send %d bytes to server.\n", len);
    if (ret < 0) {
        perror("");
    }
    icbexit();
}

/* send normal open group message to the server */

void
csendopen(char *txt)
{
    char **p;
    int i;

    p = msplit(txt, 0, MAX_INPUTSTR);
    if (!p) {
        fprintf(stderr,
                "%s[=Error=] Out of memory for splitting packet.%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (i = 0; p[i]; ++i) {
        /* construct the packet and send it */
        sprintf(pbuf, "%c", M_OPEN);
        strcat(pbuf, p[i]);
        send_packet(pp);

        /* and then log it to the buffer and logfile */
        strcpy(mbuf, p[i]);
        putl(mbuf, PL_BUF | PL_LOG);
    }

    msplit_free(p);
}

/* send a login packet */

void
sendlogin(char *id, char *nick, char *group, char *command, char *passwd)
{

    sprintf(pbuf, "%c%s\001%s\001%s\001%s\001%s",
            M_LOGIN, id, nick, group, command, passwd);
    send_packet(pp);
}

/* send a pong */
void
sendpong()
{
    pbuf[0] = M_PONG;
    pbuf[1] = '\0';
    send_packet(pp);
}

/* send a command to the server */

void
sendcmd(char *cmd, char *args)
{
    char *nick;
    char **p;
    int i;
    int maxlen;

    if (strlen(cmd) > MAX_INPUTSTR - 5) {
        fprintf(stderr,
                "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    if (!args || *args == '\0') {
        sprintf(pbuf, "%c%s\001", M_COMMAND, cmd);
        send_packet(pp);
        return;
    }

    maxlen = MAX_INPUTSTR - strlen(cmd) - 1;

    /* special case commands that take a nickname */
    if ((strcasecmp(cmd, "write") == 0) || (strcasecmp(cmd, "m") == 0)) {
        nick = args;
        args = findspace(args);
        maxlen = maxlen - strlen(nick) - 1;
    } else
        nick = NULL;

    p = msplit(args, 0, maxlen);
    if (!p) {
        fprintf(stderr,
                "%s[=Error=] Out of memory to split packet%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (i = 0; p[i]; ++i) {
        sprintf(pbuf, "%c%s\001", M_COMMAND, cmd);
        if (nick) {
            strcat(pbuf, nick);
            strcat(pbuf, " ");
        }
        strcat(pbuf, p[i]);
        send_packet(pp);
    }

    msplit_free(p);
}

/* Send a simple command to the icb server, no splitting or
   anything fancy. */

void
send_command(char *cmd, char *arg)
{
    if (MAX_INPUTSTR < strlen(cmd) + 1 + strlen(arg)) {
        fprintf(stderr,
                "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    sprintf(pbuf, "%c%s\001%s", M_COMMAND, cmd, arg);
    send_packet(pp);
}

/* Line-split text and send each line as a command. */

void
send_split_command(char *cmd, char *who, char *text)
{
    char **frags;
    int k;
    int n;

    n = MAX_INPUTSTR - strlen(cmd) - 1 - strlen(who) - 1;
    if (n < 0) {
        fprintf(stderr,
                "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    frags = msplit(text, 0, n);
    if (frags == NULL) {
        fprintf(stderr,
                "%s[=Error=] Out of memory to split packet%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (k = 0; frags[k] != NULL; ++k) {
        sprintf(pbuf, "%c%s\001%s %s", M_COMMAND, cmd, who, frags[k]);
        send_packet(pp);
    }

    msplit_free(frags);
}

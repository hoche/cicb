/* $Id: send.c,v 1.22 2009/03/23 07:42:06 hoche Exp $ */

/* send various messages to the server */

#include "icb.h"

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

extern char *findspace();

/* Send an icb packet to the server. */

static void send_packet(char *pkt) {
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

    *(unsigned char *) pkt = len;
    ++len;

    /* Send it. */

#ifdef HAVE_OPENSSL
    if (m_ssl_on) {
        int ssl_retries = 0;
        const int ssl_max_retries = 50;

        do {
            ret = SSL_write(ssl, pkt, len);

            if (ret <= 0) {
                int ssl_error = SSL_get_error(ssl, ret);

                if (ssl_error == SSL_ERROR_WANT_READ ||
                    ssl_error == SSL_ERROR_WANT_WRITE) {
                    /* Need to wait for I/O - in blocking mode this should retry */
                    if (++ssl_retries > ssl_max_retries) {
                        fprintf(stderr,
                                "SSL write failed: too many retries.\n");
                        icbexit();
                        return;
                    }
                    continue;
                } else if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                    fprintf(stderr,
                            "SSL connection closed by peer during write.\n");
                    icbexit();
                    return;
                } else {
                    /* Fatal SSL error */
                    char err_buf[256];
                    unsigned long err = ERR_get_error();
                    if (err != 0) {
                        ERR_error_string_n(err, err_buf, sizeof(err_buf));
                        fprintf(stderr, "SSL write error: %s\n", err_buf);
                    } else {
                        fprintf(stderr, "SSL write error: %d\n", ssl_error);
                    }
                    icbexit();
                    return;
                }
            } else {
                break; /* Success */
            }
        } while (ret < 0);
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

void csendopen(char *txt) {
    char **p;
    int i;

    p = msplit(txt, 0, MAX_INPUTSTR);
    if (! p) {
        fprintf(stderr, "%s[=Error=] Out of memory for splitting packet.%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (i = 0; p[i]; ++i) {
        /* construct the packet and send it */
        size_t pbuf_size = PACKET_BUF_SIZE - 1; /* pbuf = packetbuffer + 1 */
        pbuf[0] = M_OPEN;
        pbuf[1] = '\0';
        if (safe_strncat(pbuf, p[i], pbuf_size) != 0) {
            fprintf(stderr, "%s[=Error=] Message too long after splitting.%s",
                    printcolor(ColERROR, ColSANE),
                    printcolor(ColSANE, ColSANE));
            msplit_free(p);
            return;
        }
        send_packet(pp);

        /* and then log it to the buffer and logfile */
        safe_strncpy(mbuf, p[i], MESSAGE_BUF_SIZE);
        putl(mbuf, PL_BUF | PL_LOG);
    }

    msplit_free(p);
}

/* send a login packet */

void sendlogin(char *id, char *nick, char *group, char *command, char *passwd) {
    size_t pbuf_size = PACKET_BUF_SIZE - 1;
    size_t needed = strlen(id) + strlen(nick) + strlen(group) +
                    strlen(command) + strlen(passwd) +
                    6; /* M_LOGIN + 5 separators + null */

    if (needed > pbuf_size) {
        fprintf(stderr, "Login packet too large.\n");
        return;
    }

    snprintf(pbuf, pbuf_size, "%c%s\001%s\001%s\001%s\001%s", M_LOGIN, id, nick,
             group, command, passwd);
    send_packet(pp);
}

/* send a pong */
void sendpong() {
    pbuf[0] = M_PONG;
    pbuf[1] = '\0';
    send_packet(pp);
}

/* send a command to the server */

void sendcmd(char *cmd, char *args) {
    char *nick;
    char **p;
    int i;
    int maxlen;

    if (strlen(cmd) > MAX_INPUTSTR - 5) {
        fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    if (! args || *args == '\0') {
        size_t pbuf_size = PACKET_BUF_SIZE - 1;
        size_t needed = strlen(cmd) + 3; /* M_COMMAND + cmd + \001 + null */
        if (needed > pbuf_size) {
            fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                    printcolor(ColERROR, ColSANE),
                    printcolor(ColSANE, ColSANE));
            return;
        }
        snprintf(pbuf, pbuf_size, "%c%s\001", M_COMMAND, cmd);
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
    if (! p) {
        fprintf(stderr, "%s[=Error=] Out of memory to split packet%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (i = 0; p[i]; ++i) {
        size_t pbuf_size = PACKET_BUF_SIZE - 1;
        size_t needed;

        snprintf(pbuf, pbuf_size, "%c%s\001", M_COMMAND, cmd);
        needed = strlen(pbuf);

        if (nick) {
            needed += strlen(nick) + 1; /* nick + space */
            if (needed > pbuf_size - 1 ||
                safe_strncat(pbuf, nick, pbuf_size) != 0) {
                fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                        printcolor(ColERROR, ColSANE),
                        printcolor(ColSANE, ColSANE));
                msplit_free(p);
                return;
            }
            if (safe_strncat(pbuf, " ", pbuf_size) != 0) {
                msplit_free(p);
                return;
            }
        }

        needed += strlen(p[i]);
        if (needed > pbuf_size - 1 ||
            safe_strncat(pbuf, p[i], pbuf_size) != 0) {
            fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                    printcolor(ColERROR, ColSANE),
                    printcolor(ColSANE, ColSANE));
            msplit_free(p);
            return;
        }
        send_packet(pp);
    }

    msplit_free(p);
}

/* Send a simple command to the icb server, no splitting or
   anything fancy. */

void send_command(char *cmd, char *arg) {
    size_t pbuf_size = PACKET_BUF_SIZE - 1;
    size_t needed =
        strlen(cmd) + strlen(arg) + 3; /* M_COMMAND + cmd + \001 + arg + null */

    if (needed > pbuf_size) {
        fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    snprintf(pbuf, pbuf_size, "%c%s\001%s", M_COMMAND, cmd, arg);
    send_packet(pp);
}

/* Line-split text and send each line as a command. */

void send_split_command(char *cmd, char *who, char *text) {
    char **frags;
    int k;
    int n;

    n = MAX_INPUTSTR - strlen(cmd) - 1 - strlen(who) - 1;
    if (n < 0) {
        fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    frags = msplit(text, 0, n);
    if (frags == NULL) {
        fprintf(stderr, "%s[=Error=] Out of memory to split packet%s",
                printcolor(ColERROR, ColSANE), printcolor(ColSANE, ColSANE));
        return;
    }

    for (k = 0; frags[k] != NULL; ++k) {
        size_t pbuf_size = PACKET_BUF_SIZE - 1;
        size_t needed =
            strlen(cmd) + strlen(who) + strlen(frags[k]) +
            4; /* M_COMMAND + cmd + \001 + who + space + frag + null */
        if (needed > pbuf_size) {
            fprintf(stderr, "%s[=Error=] Command too long.%s\n",
                    printcolor(ColERROR, ColSANE),
                    printcolor(ColSANE, ColSANE));
            msplit_free(frags);
            return;
        }
        snprintf(pbuf, pbuf_size, "%c%s\001%s %s", M_COMMAND, cmd, who,
                 frags[k]);
        send_packet(pp);
    }

    msplit_free(frags);
}

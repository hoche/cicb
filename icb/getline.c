/* $Id: getline.c,v 1.39 2009/04/04 09:08:40 hoche Exp $ */

#include "icb.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

#include <readline/readline.h>  /* after stdio.h */
#ifdef HAVE_READLINE_HISTORY_H
#    include <readline/history.h>
#endif

/* When this is non-zero, we skip past [=More=] prompts
   until no more output is pending, and then put this char
   in the keyboard input stream. */

static int pause_skip_char = 0;

/* Cbuf for the server.  XXX This really should be
   initialized by connecttoport or something. */

static struct Cbuf server_buf = {
    "", NULL, 1, 0, 0
};

int
readpacket(int fd, struct Cbuf *p)
{
    register int ret;

    if (p->new) {
        /* starting a new command */
        p->rptr = p->buf;
        /* read the length of the command packet */
#ifdef HAVE_SSL
        if (m_ssl_on) {
            ret = SSL_read(ssl, p->rptr, 1);
        } else {
            ret = read(fd, p->rptr, 1);
        }
#else
        ret = read(fd, p->rptr, 1);
#endif
        if (ret < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                return (0);
            } else {
                return (-1);
            }
        }
        if (!ret)
            return (-2);
        p->size = p->remain = *(p->rptr);
        p->rptr++;
        p->new = 0;
    }

    /* read as much of the command as we can get */
#ifdef HAVE_SSL
    if (m_ssl_on) {
        ret = SSL_read(ssl, p->rptr, p->remain);
    } else 
#endif
    ret = read(fd, p->rptr, p->remain);

    if (ret < 0) {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
            return (0);
        } else {
            return (-1);
        }
    }
    if (!ret)
        return (-2);

    /* advance read pointer */
    p->rptr += ret;

    /* see if we read the whole thing */
    if ((p->remain -= ret) == 0) {
        /* yes */
        *p->rptr = '\0';
        p->new = 1;
        return (1);
    } else {
        /* command still incomplete */
        return (0);
    }
}

/* Eat some data from the server, and maybe dispatch it. */
void
read_from_server(void)
{
    int saved_len = 0;

    switch (readpacket(port_fd, &server_buf)) {
    case -1:                   /* error */
    case -2:                   /* lost connection */
        putl("Lost connection with the server. Quitting...", PL_ALL);
        icbexit();
        break;
    case 0:                    /* incomplete packet */
        break;
    case 1:                    /* complete packet */
        if (rl_end) {
            saved_len = rl_end;
            rl_end = 0;
            rl_redisplay();
        }
        /* reset any keepalive timers */
        alarm(gv.keepalive);
        dispatch(server_buf.buf + 1);
        if (saved_len) {
            rl_end = saved_len;
            rl_redisplay();
        }
        break;
    default:
        abort();
    }
}

/* Return a char of input to readline.  Also check for
   traffic from the server socket and dispatch that if
   necessary. */

int
getc_or_dispatch(FILE * fp)
{
    fd_set read_fds;
    fd_set r_fds;
    int max_fd;
    int ret;
    int fd;
    char buf[1];
    int port_fd_available = 0;

    FD_ZERO(&read_fds);

    fd = fileno(fp);
    FD_SET(fd, &read_fds);
    max_fd = fd;

    /* Listen to the server only if the input line is empty,
       or if we're in 'async' mode and SSL is off. */
#ifdef HAVE_SSL
    if (rl_end == 0 || (gv.asyncread && !m_ssl_on)) {
#else
    if (rl_end == 0 || gv.asyncread) {
#endif
        FD_SET(port_fd, &read_fds);
        max_fd = (max_fd < port_fd) ? port_fd : max_fd;
    }

    for (;;) {
        static struct timeval zerot = { 0, 0 };
        struct timeval *tv;

        /* If we're skipping pauses, return immediately
           since pause_skip_char is part of keyboard input. */
        tv = pause_skip_char ? &zerot : NULL;

        r_fds = read_fds;

        /* Test if we can read keyboard input (and from server if SSL is off). */
        ret = select(max_fd + 1, &r_fds, 0, 0, tv);

        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                abort();
            }
        }

        /* Test if SSL data is available. */
#ifdef HAVE_SSL
        if (m_ssl_on) {
            port_fd_available = SSL_peek(ssl, buf, 1) == 1;
        } else
#endif
        /* Test if we can read from server. */
        port_fd_available = (FD_ISSET(port_fd, &r_fds));

        if (port_fd_available) {
            read_from_server();
            if (pause_skip_char) {
                /* Skip the keyboard, keep spewing output. */
                continue;
            }
        }

        if (pause_skip_char) {
            int c = pause_skip_char;
            pause_skip_char = 0;
            return c;
        }

        if (FD_ISSET(fd, &r_fds)) {
            return rl_getc(fp);
        }
    }
}

char *
__getline(char *prompt)
{
    char *line = NULL;
    int eofcount = 0;

    /* Typing ctrl-d (delete-forward) at an empty line causes readline to
       return eof, which is probably not what the user wanted.  However,
       ignoring eof means we end up spinning cpu if we do encounter a real
       eof.  So, we treat N consecutive eofs as a real eof.

       XXX a better fix would be to disable the ctrl-d eof semantics, but
       that requires saving and setting the tty state before calling
       readline, whcih is complicated by portability issues, so we use this
       simple workaround for now. */

    while ((line = readline(prompt)) == NULL) {
        if (100 < ++eofcount) {
            icbexit();
        }
        askquit();
        if (gv.cute) {
            saysomething();
        }
    }

    /* reset pagination counter. */
    linenumber = 0;

    /* zero rl_end so that pauseprompt() will pause. */
    rl_end = 0;

#ifdef HAVE_READLINE_HISTORY_H
    if (*line != '\0') {
        add_history(line);
    }
#endif

    if (*line == '\0' && gv.cute) {
        saysomething();
    }

    return line;
}

/* Note the \r at the end of spaces[]. */
static char spaces[] = "                                  \r";
static int n_spaces = sizeof(spaces) - 1 - 1;

/* print a prompt, and wait for a character */
/* if Erase is nonzero, the prompt is Erased by backspacing */
/* if c is non-null, use must type c to continue */
/* if unget it set, user's character is pushed back into input buffer, unless
   it is one of the characters in except. */

void
pauseprompt(char *prompt, int Erase, int c, int unget, char *except)
{
    extern int _rl_meta_flag;   /* XXX private var, sigh. */
    char uc;

    /* If we're not interactive, don't pause.  If you remove this, then
       you need to make sure readlineinit gets called too. */
    if (!gv.interactive) {
        return;
    }

    /* If user typed something, don't pause. */
    if (pause_skip_char) {
        return;
    }

    /* If user is typing, don't pause. */
    if (gv.asyncread && rl_end != 0) {
        return;
    }

    /* print the prompt */
    write(1, prompt, strlen(prompt));

    /* we may be called from outside readline, so make sure we're in
       cbreak mode for this.  we don't bother undoing cbreak mode,
       because all the escapes from icb already will. */

    rl_prep_term_function(_rl_meta_flag);

    do {
        uc = rl_getc(stdin);
    } while (c != '\0' && uc != c);

    if (Erase) {
        int n = strlen(prompt) + 1;

        write(1, "\r", 1);
        while (n_spaces < n) {
            write(1, spaces, n_spaces);
            n -= n_spaces;
        }
        /* write spaces and a \r */
        write(1, spaces + n_spaces - n, n + 1);

    } else {
        write(1, "\r\n", 2);
    }

    /* push character back onto stream if requested */
    if (unget && (!except || strchr(except, uc) == 0)) {
        pause_skip_char = uc;
    }
}

/*
 *  keystroke handler for tab key
 */
void
handletab(int count, char c)
{
    char mpref[256];
    int ppoint;
    int words;
    int term;
    int i;
    int word2, word2len, diff;
    char find_nick[MAX_NICKLEN];
    char *found_nick;
    char *histmatch();

    /* make sure the history's not empty - punt if it is */
    if (histcount() == 0) {
        return;
    }

    /* remember cursor location */
    ppoint = rl_point;

    /* determine case */
    if (rl_end == 0 || rl_line_buffer[0] != gv.cmdchar) {
        /* case 1 - no command char */
        if ((found_nick = histget()) != 0) {
            sprintf(mpref, "%cm", gv.cmdchar);
            strcat(mpref, " ");
            strcat(mpref, found_nick);
            strcat(mpref, " ");
            rl_point = 0;
            rl_insert_text(mpref);
            rl_point = rl_end;
            rl_point = ppoint + strlen(mpref);
            putchar('\r');
            rl_forced_update_display();
        } else
            printf("\007");
        return;
    }

    /* starts with command character */
    words = 0;
    term = 0;
    word2 = 0;
    word2len = -1;
    if (rl_end) {
        words++;
        for (i = 0; i < rl_end; ++i) {
            if (isspace(rl_line_buffer[i])) {
                term++;
                if (words == 2)
                    word2len = i - word2;
            } else if (term) {
                term = 0;
                words++;
                if (words == 2)
                    word2 = i;

            }
        }
        if (word2len < 0)
            word2len = rl_end - word2;
    }

    /* case 3 - add nickname */
    if ((words == 1) && term && rl_point == rl_end) {
        if ((found_nick = histget()) != 0) {
            rl_insert_text(histget());
            rl_insert(1, ' ');
            putchar('\r');
            rl_forced_update_display();
        } else
            printf("\007");
        return;
    }

    /* case 4 - nickname completion */
    if ((words == 2) && !term && rl_point == rl_end) {
        if (word2len > MAX_NICKLEN) {
            printf("\007");
            return;
        }

        if (rl_line_buffer[word2] == '@') {
            word2++;
            word2len--;
        }

        for (i = word2; i < rl_end; ++i) {
            find_nick[i - word2] = rl_line_buffer[i];
        }
        find_nick[i - word2] = '\0';
        found_nick = histmatch(find_nick);
        if (found_nick) {
#ifdef HAVE_READLINE_2
            rl_rubout(word2len);
#else
            rl_rubout(word2len, 0);
#endif
            rl_insert_text(found_nick);
            if (match_exact)
                rl_insert(1, ' ');
            else
                printf("\007");
            putchar('\r');
            rl_forced_update_display();
        } else
            printf("\007");
        return;
    }

    /* case 5 - replace nickname */
    if (words >= 2) {
        found_nick = histget();
        if (found_nick) {
            rl_point = word2;
#ifdef HAVE_READLINE_2
            rl_delete(word2len);
#else
            rl_delete(word2len, 0);
#endif
            rl_insert_text(found_nick);
            rl_point = rl_end;
#ifdef HAVE_READLINE_2
            rl_kill_line(1);
#else
            rl_kill_line(1, 0);
#endif
            diff = strlen(found_nick) - word2len;
            rl_point = ppoint + diff;
            if (diff < 0) {     /* line shrunk */
                rl_point = rl_end;
                rl_insert(-diff, ' ');
                putchar('\r');
                rl_forced_update_display();
#ifdef HAVE_READLINE_2
                rl_rubout(-diff);
#else
                rl_rubout(-diff, 0);
#endif
            }
            putchar('\r');
            rl_forced_update_display();
        } else
            printf("\007");
        return;
    }

    /* anything else */
    printf("\007");
}

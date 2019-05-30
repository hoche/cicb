/* $Id: main.c,v 1.53 2009/04/04 09:08:40 hoche Exp $ */

#include "icb.h"
#include "getswitch.h"

#include <readline/readline.h>

#ifdef HAVE_SSL
#    include <openssl/crypto.h>
#    include <openssl/x509.h>
#    include <openssl/pem.h>
#    include <openssl/ssl.h>
#    include <openssl/err.h>
#if (SSLEAY_VERSION_NUMBER >= 0x0907000L)
#    include <openssl/conf.h>
#endif
#endif

char *optv[] = {
    "clear",
    "list",
    "who",
    "restricted",
    "nickname:",
    "group:",
    "host:",
    "port:",
    "password:",
    "server:",
    "bindhost:",
#ifdef HAVE_SSL
    "SSL",
#endif
    "color",
    "help/",
    (char *)NULL
};

#ifdef HAVE_SSL
int init_openssl_library(void)
{
    #if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    #else
    OPENSSL_init_ssl(0, NULL);
    #endif

    SSL_load_error_strings();
    /*OPENSSL_config(NULL); */

    OpenSSL_add_ssl_algorithms();

    return(1);
}
#endif

void
usage(char *name, int ret)
{
    fprintf(stderr, "Usage: %s [switches]\n", name);
    fprintf(stderr, " Switches may be abbreviated.  They are:\n");
    fprintf(stderr, "  -nickname nick\tsign on with nickname nick.\n");
    fprintf(stderr, "  -password pass\tset password to pass.\n");
    fprintf(stderr, "  -password -\t\tprompt for password.\n");
    fprintf(stderr, "  -group group\t\tsign on into group group.\n");
    fprintf(stderr, "  -server name\t\tconnect to server named name.\n");
    fprintf(stderr, "  -host host\t\tconnect to server on host host.\n");
    fprintf(stderr, "  -port port\t\ttry to connect to port port.\n");
    fprintf(stderr,
            "  -bindhost host\t\ttry to bind outgoing connect to host host.\n");
#ifdef HAVE_SSL
    fprintf(stderr, "  -SSL\t\tuse SSL.\n");
#endif
    fprintf(stderr, "  -list\t\t\tlist known servers, in order.\n");
    fprintf(stderr, "  -clear\t\twipe args from command line.\n");
    fprintf(stderr, "  -who\t\t\tsee who's on; don't sign on.\n");
    fprintf(stderr, "  -restricted\t\trestricted mode\n");
    fprintf(stderr,
            "  -color\t\tenable ANSI color mode (disabled by default)\n");
    fprintf(stderr, "  -help\t\t\tdisplay this message.\n");
    fprintf(stderr,
            " Note: specifying a password on the command line implies -clear.\n");
    exit(ret);
}

/* The main read/eval/print loop.  Nothing fancy here. */

static void
main_loop(void)
{
    char *line;
    char prompt[1];

    prompt[0] = 0;
    while ((line = __getline((char *)&prompt)) != NULL) {
        parse(line);
        free(line);
    }
}

int
main(int argc, char *argv[])
{
    char *s;
    int restrictflg = 0;
    int clearargsflg = 0;
    int listflg = 0;
    int use_server_nick = 0;
    char *bindhost = (char *)NULL;

    myserver = NULL;
    mygroup = strcpy(group, "1");
    mynick =
        (getenv("ICBNAME") ? strncpy(nick, getenv("ICBNAME"), MAX_NICKLEN + 1) :
         NULL);
    nick[MAX_NICKLEN] = '\0';
    myport = DEFAULT_PORT;

    m_ssl_on = 0;

    while ((s = getswitch(argc, argv, optv)) != NULL) {
        switch (s[0]) {

        case 'b':
            if (s[1] == 'i')
                bindhost = strdup(switcharg);
            else
                usage(argv[0], 0);
            break;

        case 'c':
            switch (s[1]) {
            case 'l':
                clearargsflg++;
                break;
            case 'o':
                gv.colorize = 1;
                break;
            }
            break;

        case 'g':
            strncpy(group, switcharg, MAX_NICKLEN + 1);
            group[MAX_NICKLEN] = '\0';
            mygroup = group;
            break;

        case 'h':
            if (s[1] == 'o')
                myserver = strdup(switcharg);
            else
                usage(argv[0], 0);
            break;

        case 'l':
            listflg++;
            gv.interactive = 0;
            break;

        case 'n':
            strncpy(nick, switcharg, MAX_NICKLEN + 1);
            nick[MAX_NICKLEN] = '\0';
            mynick = nick;
            break;

        case 'p':
            switch (s[1]) {
            case 'o':
                myport = atoi(switcharg);
                break;
            case 'a':
                strncpy(pass, switcharg, MAX_PASSLEN);
                pass[MAX_PASSLEN] = '\0';
                mypass = pass;
                if (pass[0] == '-') {
                    mypass = getpass("Password:");
                }
                clearargsflg++;
                break;
            }
            break;

        case 'r':
            restrictflg = 1;
            break;

        case 's':
            myserver = strdup(switcharg);
            use_server_nick = 1;
            break;

        case 'S':
            m_ssl_on = 1;
#ifndef HAVE_SSL
            fprintf(stderr, "This client not compiled for SSL.");
#endif
            break;

        case 'w':
            whoflg++;
            gv.interactive = 0;
            break;

        case '?':
        default:
            usage(argv[0], 1);
            break;
        }
    }

    if (listflg) {
        listservers();
        exit(0);
    }

    if (clearargsflg)
        clearargs(argc, argv);

    /* PLEASE DO NOT FAKE LOGIN IDS - IT IS AGAINST ICB RULES */
    if ((myloginid = getloginid()) == NULL) {
        fprintf(stderr, "Error - can't determine your login ID.");
        exit(-1);
    }

    /* get our nickname */
    if (mynick == NULL && (mynick = getenv("ICBNAME")) == NULL &&
        (mynick = getenv("FNNAME")) == NULL &&
        (mynick = getenv("FORUMNAME")) == NULL)
        mynick = myloginid;

    /* can you believe this program uses random numbers? */
    srand((unsigned)time(0));

    /* initialize everybody and say hello. */
    write(2, icb_version, strlen(icb_version));
    write(2, "\n", 1);
    write(2, "Built with readline ", 20);
    write(2, rl_library_version, strlen(rl_library_version));
    write(2, " and TCL ", 9);
    write(2, TCL_PATCH_LEVEL, strlen(TCL_PATCH_LEVEL));
    write(2, ".\n", 2);

    getwinsize();

    if (gv.interactive) {
        tcl_init();
#ifndef HAVE_READLINE_2
        /* we'll install our own signal handlers */
        rl_catch_signals = 0;
        /* we'll call readline's resizer ourselves when appropriate */
        rl_catch_sigwinch = 0;
#endif
        readlineinit();
        if (restrictflg && !gv.restricted)
            set_restricted();
    }
#ifdef HAVE_SSL
    init_openssl_library();
    if (m_ssl_on && myport == DEFAULT_PORT)
        myport = DEFAULT_SSL_PORT;
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
#if HAVE_TLS_CLIENT_METHOD
    ctx = SSL_CTX_new(TLS_client_method());
#else
    ctx = SSL_CTX_new(TLSv1_1_client_method());
#endif
    if (!ctx) {
        fprintf(stderr, "Error setting up the SSL context.\n");
        exit(1);
    }
#endif

    if (myserver == NULL)
        myserver = strdup(DEFAULT_HOST);

    if ((port_fd =
         connect_to_server(use_server_nick, myserver, myport, bindhost)) < 0) {
        fprintf(stderr, "%s: %s: server not found.\n", argv[0], myserver);
        if (use_server_nick) {
            fprintf(stderr,
                    "(use -h to specify a hostname instead of a server name from the icbserverdb file)\n");
        } else {
            fprintf(stderr,
                    "(use -s to specify a server name from the icbserverdb file instead of a host name)\n");
        }
        exit(1);
    }
#ifdef HAVE_SSL
    if (m_ssl_on) {
        int result, ssl_error;

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, port_fd);
        result = SSL_connect(ssl);
        switch (ssl_error) {
            /* not sure how to handle a bunch of these cases, so we just report
             * 'em and go on for now */
        case SSL_ERROR_WANT_READ:
            fprintf(stderr, "SSL_ERROR_WANT_READ at SSL_connect().\n");
            SSL_free(ssl);
            exit(1);

        case SSL_ERROR_WANT_WRITE:
            fprintf(stderr, "SSL_ERROR_WANT_WRITE at SSL_connect().\n");
            SSL_free(ssl);
            exit(1);

        case SSL_ERROR_ZERO_RETURN:
            fprintf(stderr, "SSL_ERROR_ZERO_RETURN at SSL_connect().\n");
            SSL_free(ssl);
            exit(1);

        case SSL_ERROR_WANT_X509_LOOKUP:
            fprintf(stderr, "SSL_connect() wants X509 lookup.\n");
            SSL_free(ssl);
            exit(1);

        case SSL_ERROR_SYSCALL:
            fprintf(stderr, "SSL_ERROR_SYSCALL at SSL_connect().\n");
            SSL_free(ssl);
            exit(1);

        case SSL_ERROR_SSL:
            ssl_error = ERR_get_error();
            if (ssl_error == 0) {
                if (result == 0) {
                    fprintf(stderr, "SSL_connect got bad EOF.\n");
                } else {
                    fprintf(stderr, "SSL_connect got socket I/O error.\n");
                }
            } else {
                char err_buf[256];
                ERR_error_string(ssl_error, &err_buf[0]);
                fprintf(stderr, "SSL_connect error: %s", err_buf);
            }
            SSL_free(ssl);
            exit(1);
        }
        printf("SSL connection using %s\n", SSL_get_cipher(ssl));
    }
#endif

    if (gv.interactive) {
        trapsignals();
        main_loop();
        icbexit();
    }

    for (;;) {
        read_from_server();
    }

    return 0;
}

void
icbexit()
{
    rl_deprep_term_function();

    /* close the session log */
    if (logging())
        closesessionlog();

#ifdef HAVE_SSL
    if (m_ssl_on) {
        SSL_shutdown(ssl);
        close(port_fd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
#else
    close(port_fd);
#endif

    exit(0);
}

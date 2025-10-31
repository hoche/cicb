/* $Id: main.c,v 1.53 2009/04/04 09:08:40 hoche Exp $ */

#include "icb.h"
#include "getswitch.h"

#include <readline/readline.h>
#include <time.h>

#ifdef HAVE_OPENSSL
#    include <openssl/crypto.h>
#    include <openssl/x509.h>
#    include <openssl/pem.h>
#    include <openssl/ssl.h>
#    include <openssl/err.h>
#    include <openssl/conf.h>
#    include <sys/select.h>
#    include <sys/time.h>
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
#ifdef HAVE_OPENSSL
    "SSL",
#endif
    "color",
    "help/",
    (char *)NULL
};

#ifdef HAVE_OPENSSL
/* Modern OpenSSL initialization (OpenSSL 1.1.0+) */
int init_openssl_library(void)
{
    /* OPENSSL_init_ssl() initializes the SSL library and error strings */
    if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL) == 0) {
        return 0;
    }
    
    /* Additional configuration can be done here if needed */
    return 1;
}

/* Check if socket is ready for I/O with timeout */
static int ssl_wait_for_io(SSL *ssl, int want_read, int timeout_sec)
{
    int fd = SSL_get_fd(ssl);
    fd_set read_fds, write_fds;
    struct timeval timeout, *timeout_ptr = NULL;
    int ret;

    if (timeout_sec > 0) {
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;
        timeout_ptr = &timeout;
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    if (want_read) {
        FD_SET(fd, &read_fds);
    } else {
        FD_SET(fd, &write_fds);
    }

    ret = select(fd + 1, &read_fds, &write_fds, NULL, timeout_ptr);
    
    if (ret < 0) {
        return -1;  /* Error */
    }
    if (ret == 0) {
        return 0;   /* Timeout */
    }
    return 1;       /* Ready */
}
#endif

void
usage(char *name, int ret)
{
    fprintf(stderr, "Usage: %s [switches]\n", name);
    fprintf(stderr, "  Switches may be abbreviated.  They are:\n");
    fprintf(stderr, "  -b <host>    Try to bind outgoing connect to <host>\n");
    fprintf(stderr, "  -cl          Clear args from command line\n");
    fprintf(stderr, "  -co          Use color\n");
    fprintf(stderr, "  -g <group>   Sign on into <group>\n");
    fprintf(stderr, "  -h           display this message.\n");
    fprintf(stderr, "  -l           List known servers\n");
    fprintf(stderr, "  -n <nick>    Sign on with nickname nick.\n");
    fprintf(stderr, "  -pa <pass>   Use icb password <pass>. If <pass> is\n");
    fprintf(stderr, "               empty, prompt for it.\n");
    fprintf(stderr, "  -po <port>   Try to connect to <port>.\n");
    fprintf(stderr, "  -r           Use restricted mode\n");
    fprintf(stderr, "  -s <host>    Connect to server with DNS name <host>\n");
#ifdef HAVE_OPENSSL
    fprintf(stderr, "  -S           use SSL.\n");
#endif
    fprintf(stderr, "  -w           see who's on; don't sign on.\n");
    fprintf(stderr, "  Note: specifying a password on the command line "
                    "implies -cl (clear).\n");
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
            bindhost = strdup(switcharg);
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
                {
                    int port_val;
                    if (safe_atoi(switcharg, &port_val) != 0 || port_val <= 0 || port_val > 65535) {
                        fprintf(stderr, "%s: Invalid port number: %s\n", argv[0], switcharg);
                        exit(1);
                    }
                    myport = port_val;
                }
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

/*
        // this was the old version, but people found it counterintuitive that
        // this selected from a list of canonical server nicknames rather than
        // from the actual DNS name for the server, confusing it the the -h
        // flag. Since I was annoyed that -h had been overridden and wasn't
        // being used for "help", I decided to move it and disable the
        // canonical-name-lookup thing.
        // 
        // I'm probably going to remove this whole feature unless someone
        // complains.
        // hoche - 5/30/19
        case 's':
            myserver = strdup(switcharg);
            use_server_nick = 1;
            break;
*/
        case 's':
            myserver = strdup(switcharg);
            break;


        case 'S':
            m_ssl_on = 1;
#ifndef HAVE_OPENSSL
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
        /* TCL initialization is optional - program works without it */
        if (tcl_init() != 0) {
            fprintf(stderr, "Warning: TCL initialization failed. Continuing without TCL support.\n");
        }
        /* we'll install our own signal handlers */
        rl_catch_signals = 0;
        /* we'll call readline's resizer ourselves when appropriate */
        rl_catch_sigwinch = 0;
        readlineinit();
        if (restrictflg && !gv.restricted)
            set_restricted();
    }
#ifdef HAVE_OPENSSL
    if (m_ssl_on) {
        if (init_openssl_library() == 0) {
            fprintf(stderr, "Error initializing OpenSSL library.\n");
            exit(1);
        }
        if (myport == DEFAULT_PORT)
            myport = DEFAULT_SSL_PORT;
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            fprintf(stderr, "Error setting up the SSL context.\n");
            exit(1);
        }
    }
#endif

    if (myserver == NULL)
        myserver = strdup(DEFAULT_HOST);

    if ((port_fd =
         connect_to_server(use_server_nick, myserver, myport, bindhost)) < 0) {
        fprintf(stderr, "%s: %s: server not found.\n", argv[0], myserver);
/*
        if (use_server_nick) {
            fprintf(stderr,
                    "(use -h to specify a hostname instead of a server name from the icbserverdb file)\n");
        } else {
            fprintf(stderr,
                    "(use -s to specify a server name from the icbserverdb file instead of a host name)\n");
        }
*/
        exit(1);
    }
#ifdef HAVE_OPENSSL
    if (m_ssl_on) {
        int result;
        int ssl_error;
        int retries = 0;
        const int max_retries = 100;  /* Maximum number of retry attempts */
        const int timeout_sec = 30;    /* 30 second timeout total */
        time_t start_time = time(NULL);

        ssl = SSL_new(ctx);
        if (!ssl) {
            fprintf(stderr, "Error creating SSL structure.\n");
            SSL_CTX_free(ctx);
            exit(1);
        }

        SSL_set_fd(ssl, port_fd);
        result = SSL_connect(ssl);

        /* Modern SSL connect with timeout and retry limit */
        while (result < 0 && retries < max_retries) {
            time_t current_time = time(NULL);
            
            /* Check for timeout */
            if (current_time - start_time > timeout_sec) {
                fprintf(stderr, "SSL connection timeout after %d seconds.\n", timeout_sec);
                SSL_free(ssl);
                SSL_CTX_free(ctx);
                exit(1);
            }

            ssl_error = SSL_get_error(ssl, result);

            switch (ssl_error) {
                case SSL_ERROR_WANT_READ:
                    /* Wait for socket to be readable */
                    if (ssl_wait_for_io(ssl, 1, 5) <= 0) {
                        fprintf(stderr, "SSL connection timeout waiting for read.\n");
                        SSL_free(ssl);
                        SSL_CTX_free(ctx);
                        exit(1);
                    }
                    result = SSL_connect(ssl);
                    retries++;
                    break;

                case SSL_ERROR_WANT_WRITE:
                    /* Wait for socket to be writable */
                    if (ssl_wait_for_io(ssl, 0, 5) <= 0) {
                        fprintf(stderr, "SSL connection timeout waiting for write.\n");
                        SSL_free(ssl);
                        SSL_CTX_free(ctx);
                        exit(1);
                    }
                    result = SSL_connect(ssl);
                    retries++;
                    break;

                case SSL_ERROR_ZERO_RETURN:
                    fprintf(stderr, "SSL connection closed by peer during handshake.\n");
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    exit(1);

                case SSL_ERROR_WANT_X509_LOOKUP:
                    fprintf(stderr, "SSL handshake requires X509 certificate lookup (not supported).\n");
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    exit(1);

                case SSL_ERROR_SYSCALL:
                    fprintf(stderr, "SSL connection system call error.\n");
                    if (ERR_peek_error() != 0) {
                        char err_buf[256];
                        unsigned long err = ERR_get_error();
                        ERR_error_string_n(err, err_buf, sizeof(err_buf));
                        fprintf(stderr, "OpenSSL error: %s\n", err_buf);
                    }
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    exit(1);

                case SSL_ERROR_SSL:
                    {
                        char err_buf[512];
                        unsigned long err;
                        while ((err = ERR_get_error()) != 0) {
                            ERR_error_string_n(err, err_buf, sizeof(err_buf));
                            fprintf(stderr, "SSL error: %s\n", err_buf);
                        }
                    }
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    exit(1);

                default:
                    fprintf(stderr, "Unknown SSL error: %d\n", ssl_error);
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    exit(1);
            }
        }

        if (result < 0) {
            fprintf(stderr, "SSL connection failed after %d retries.\n", retries);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            exit(1);
        }

        if (retries >= max_retries) {
            fprintf(stderr, "SSL connection exceeded maximum retries (%d).\n", max_retries);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            exit(1);
        }

        printf("SSL connection established using %s\n", SSL_get_cipher(ssl));
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

#ifdef HAVE_OPENSSL
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


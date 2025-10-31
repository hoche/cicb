/*
 * Routines for getting the server info from a server config file
 * and connecting to it.
 */

#include "icb.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

/************************************************************************
 * Utils for reading the server file
 ***********************************************************************/

/* read a line from the server file and populate serv.
 */
int readserverent(FILE *fd, struct server *serverdata) {
    char line[256];
    int index;

    if (! fd || ! serverdata) {
        return -1;
    }

    for (;;) {
        if (! fgets(line, 256, fd)) {
            return 0;
        }

        index = 0;

        while (line[index] && isspace(line[index]))
            ++index;

        if (! line[index] || line[index] == '#') {
            continue;
        }

        if ((sscanf(line, "\"%[^\"]\" %s %d", serverdata->name,
                    serverdata->host, &(serverdata->port)) != 3) &&
            (sscanf(line, "%s %s %d", serverdata->name, serverdata->host,
                    &(serverdata->port)) != 3)) {
            serverdata->port = DEFAULT_PORT;
            if ((sscanf(line, "\"%[^\"]\" %s", serverdata->name,
                        serverdata->host) != 2) &&
                (sscanf(line, "%s %s", serverdata->name, serverdata->host) !=
                 2)) {
                if (sscanf(line, "%s", serverdata->host) == 1) {
                    safe_strncpy(serverdata->name, serverdata->host,
                                 sizeof(serverdata->name));
                } else {
                    fprintf(stderr, "Bad server entry: %sSkipping...\n", line);
                    continue;
                }
            }
        }

        return 1;
    }
}

/* open the server config file
 * return NULL and set errno if it couldn't be opened
 *
 * if they have a .icbservrc in their home directory, cat it and the
 * system one together and then use the output of the cat.
 */
FILE *openserverfile() {
    struct stat statbuf;
    char command[256];
    char pwd[PATH_MAX + 1];
    FILE *ret;

    getcwd(pwd, PATH_MAX + 1);

    chdir(getenv("HOME"));

    if (! stat(PERSONALSL, &statbuf)) {
        sprintf(command, "/bin/cat %s %s\n", PERSONALSL, SERVERLIST);
        ret = popen(command, "r");
    } else
        ret = fopen(SERVERLIST, "r");

    chdir(pwd);
    return ret;
}

/* read from the server config file, looking for a named entry.
 * If it finds it, it returns a pointer to the server entry on
 * success and NULL on failure.
 */
int getserver(char *name, struct server *serverdata) {
    int ret = -1;
    FILE *serverfile;

    if (! name || ! serverdata)
        return -1;

    if ((serverfile = openserverfile()) == NULL)
        return -1;

    while (readserverent(serverfile, serverdata)) {
        if (name) {
            if (! strcmp(name, serverdata->name)) {
                ret = 0;
                break;
            } else {
                if (! strcmp(name, serverdata->host)) {
                    ret = 0;
                    myserver = strdup(serverdata->name);
                    break;
                }
            }
        } else {
            ret = 0;
            break;
        }
    }

    fclose(serverfile);
    return ret;
}

/* print out a list of the entries in the server config file.
 */
void listservers() {
    FILE *serverfile;
    struct server serverbuf;

    if ((serverfile = openserverfile()) == NULL) {
        printf("Can't open server file.\n");
        return;
    } else {
        rewind(serverfile);
    }

    printf("%-16s%-32s%5s\n", "SERVER", "HOST", "PORT");
    while (readserverent(serverfile, &serverbuf)) {
        printf("%-16s%-32s%5d\n", serverbuf.name, serverbuf.host,
               serverbuf.port);
    }
    fclose(serverfile);
}

/************************************************************************
 * Connect to the server
 ***********************************************************************/

/* Given a string, return an internet address.
 * This accepts address in the form of 
 *     num.num.num.num
 *     [num.etc]
 *     machine.dom.dom.dom
 * Returns NULL if the address couldn't be determined.
 */
struct in_addr *get_address(char *spoo) {
    static struct in_addr iaddr;
    struct hostent *hp;

    /* handle case of "[num.num.num.num]" */
    if (*spoo == '[') {
        char *p;

        spoo++;
        for (p = spoo; *p != '\0'; p++)
            if (*p == ']') {
                *p = '\0';
                break;
            }
    }

    /* handle case of "num.num.num.num" */
    if (*spoo >= '0' && *spoo <= '9') {
        if ((iaddr.s_addr = (unsigned long) inet_addr(spoo)) == -1)
            return (NULL);
        return (&iaddr);
    }

    /* handle a symbolic address */
    if ((hp = gethostbyname(spoo)) == (struct hostent *) 0)
        return (NULL);

    /* copy address into inet address struct */
    memcpy(&iaddr.s_addr, hp->h_addr, hp->h_length);

    return ((struct in_addr *) &iaddr);
}

/* Connect to a server port.
 * Return a file descriptor on success
 * Return -1 on fail
 */
int server_connect(char *host_name, int port_number, char *bind_host) {
    int s;
    struct sockaddr_in saddr;
    struct in_addr *addr;

    fprintf(stderr, "Trying to connect to port %d of host %s.\n", port_number,
            host_name);
    myhost = strdup(host_name);

    /* get the client host inet address */
    if ((addr = get_address(host_name)) == NULL) {
        fprintf(stderr, "icb: can't get address of %s\n", host_name);
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_addr.s_addr = addr->s_addr;

    /* fill in socket domain and port number */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((in_port_t) port_number);

    /* create a socket */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "icb: couldn't connect to %s server.\n", host_name);
        return -1;
    }

    if (bind_host != NULL) {
        struct sockaddr_in bind_saddr;

        if ((addr = get_address(bind_host)) == NULL) {
            fprintf(stderr, "icb: can't get address of %s\n", bind_host);
            return -1;
        }

        /* insert bind_host hostname into address */
        memset(&bind_saddr, '\0', sizeof(bind_saddr));
        bind_saddr.sin_addr.s_addr = addr->s_addr;

        /* fill in socket domain */
        bind_saddr.sin_family = AF_INET;

        if (bind(s, (struct sockaddr *) &bind_saddr, sizeof bind_saddr) < 0) {
            fprintf(stderr, "icb: can't bind socket to %s\n", bind_host);
            return -1;
        }
    }

    /* Try to set keepalive.  No big deal if it fails. */

#ifdef SO_KEEPALIVE
    {
        int val = 1;
        if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof val) < 0) {
            fprintf(stderr, "Warning: can't set socket options: %s\n",
                    strerror(errno));
        } else {
            fprintf(stderr, "TCP Keepalives ON\n");
        }
    }
#endif

    /* connect it to the server inet address */
    if (connect(s, (struct sockaddr *) &saddr, sizeof saddr) < 0) {
        fprintf(stderr, "icb: can't connect to %s.\n", host_name);
        return -1;
    }

    /* make it non-blocking */
#ifdef O_NONBLOCK
    if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
        return -1;
    }
#endif

    return s;
}

/************************************************************************
 * Main entry point
 ***********************************************************************/

/* look up server in the server file list and connect to it.
 * returns the file descriptor of the socket to the
 * server on success, -1 on error.
 */

int connect_to_server(int use_nick, char *name, int port, char *bind_host) {
    struct server serverdata;

    if (use_nick) {
        if (getserver(name, &serverdata) < 0)
            return -1;
        else {
            name = serverdata.host;
            port = serverdata.port;
        }
    } else {
        if (! name) {
            /* if there is no name, then get the default server */
            if (getserver(NULL, &serverdata) < 0) {
                name = DEFAULT_HOST;
                getserver(name, &serverdata);
            }
        }
        port = (port ? port : DEFAULT_PORT);
    }

    return server_connect(name, port, bind_host);
}

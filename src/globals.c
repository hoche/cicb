/* $Id: globals.c,v 1.30 2009/03/23 07:42:06 hoche Exp $ */

/* icb global variables */

#include "icb.h"
#include "version.h"

int port_fd;

/* non-global definitions */
char packetbuffer[PACKET_BUF_SIZE]; /* packet buffer */
char messagebuffer[MESSAGE_BUF_SIZE];   /* generic large buffer */

char nick[MAX_NICKLEN + 1];
char group[MAX_NICKLEN + 2];    /* to allow room for @user */
char pass[MAX_PASSLEN + 1];

/* global defs */
char *what_version = WHAT_VERSION;  /* ICB Client Version */
char *pp = packetbuffer;        /* packet pointer */
char *pbuf = packetbuffer + 1;  /* packet buffer pointer */
char *mbuf = messagebuffer;     /* message buffer */
char *mynick = NULL;            /* nickname of user */
char *myloginid = 0;            /* login id of user */
char *mypass = "";              /* password of user */
char *mygroup = "";             /* requested group */
char *myserver = NULL;          /* name of server */
char *myhost = NULL;            /* hostname of server */
char *personalhilite = NULL;    /* personal highlight list. malloc */
                                /* with Tcl_Alloc if need be */
char extendir[255];             /* directory for tcl extensions */
char *rcdir = NULL;

int myport = 0;                 /* string form of port number */
int beeping = 0;                /* 1 if beeping turned on */
int connected = 0;              /* 1 when server validates us */
char thishost[132];             /* our hostname */
char continued = 0;             /* did we bg then fg this job? */
int whoflg = 0;                 /* just do a who at startup */
time_t curtime;                 /* current time */
int linenumber = 0;             /* current screen line number */
int match_exact = 0;            /* used by histmatch() */

int m_ssl_on = 0;

#ifdef HAVE_OPENSSL
SSL_CTX *ctx = NULL;
SSL *ssl = NULL;
#endif

/* flags set in .forumrc */
int m_whoheader = 1;            /* who header output */
int m_groupheader = 1;          /* group header output */
int m_nomesg = 0;               /* turn off messages while in forum */
int m_watchtime = 0;            /* using boring time format */

GLOBS gv = {
    CMDCHAR,                    /* cmdchar */
    0,                          /* verifyquit */
    0,                          /* pauseonshell */
    1,                          /* interactive */
    -1,                         /* page length */
    -1,                         /* page width */
    15,                         /* phistory */
    250,                        /* bufferlines */
    0,                          /* groupblanks */
    "icb.log",                  /* logfile */
    0,                          /* restricted */
    "normal",                   /* timedisplay */
    1,                          /* beeps */
    "vi",                       /* editmode */
    0,                          /* tabreply */
    1,                          /* cute */
    0,                          /* autodel */
    "",                         /* personalto */
    0,                          /* logreplay */
    0,                          /* keepalive */
    0,                          /* mutepongs */
    "none",                     /* alert */
    0,                          /* autoregister */
    0,                          /* asyncread */
    0,                          /* colorize */
    0                           /* message timestamp on/off */
};

struct COLORTABLE colortable[] = {
    {"\x1b[0m", 1},             /* sane : no colors */
    {"\x1b[0m", 0},             /* normal : standard color */
    {"\x1b[0m", 0},             /* notice : status messages */
    {"\x1b[0m", 0},             /* warning : warning messages */
    {"\x1b[0m", 0},             /* error : error messages */
    {"\x1b[0m", 0},             /* nickname : nicknames */
    {"\x1b[0m", 0},             /* address : email addresses */
    {"\x1b[0m", 0},             /* idletime : idle time */
    {"\x1b[0m", 0},             /* logintime : login time */
    {"\x1b[0m", 0},             /* persfrom : personal message nick */
    {"\x1b[0m", 0},             /* personal : personal message text */
    {"\x1b[0m", 0},             /* beep : beep message */
    {"\x1b[0m", 0},             /* mod : mod star */
    {"\x1b[0m", 0},             /* unreg : (nr) flag on unreged nick */
    {"\x1b[0m", 0},             /* abrkt : angle brackets around nicks */
    {"\x1b[0m", 0},             /* sbrkt : square brackets around server messages */
    {"\x1b[0m", 0},             /* pbrkt : personal message nick brackets */
    {"\x1b[0m", 0},             /* whead : header line of /who listing */
    {"\x1b[0m", 0},             /* wsub : subheader line of /who listing */
    {"\x1b[0m", 0},             /* more : the MORE prompt */
    {"\x1b[0m", 0},             /* status : server status messages (unused so far) */
    {"\x1b[0m", 0},             /* pershilite : personal highlighting */
    {"\x1b[0m", 0},             /* persfromhilite : personal from hilite */
    {"\x1b[0m", 0},             /* timestamp : message timestamp */
};

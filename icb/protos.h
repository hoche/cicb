/*
	This used to be an automatically generated file.

	If you edit this file, your changes may be lost later.
*/
#include "config.h"

STRLIST *strgetnode(char *str, STRLIST *head, int caseindep);
STRLIST *strmakenode(int size);
char *ampm(time_t secs, int style);
char *findspace(char *s);
char *getloginid(void);
char *getword(char *s);
char *histget(void);
char *idlestr(char *num);
char *nextword(char *s);
char *response(char *num);
char *tildexpand(char *s);
int addusage(char *name, int type, char *args, char *usage);
void askquit();

/* protos for tcl handlers */

int c_color (ARGV_TCL);
int c_hclear (ARGV_TCL);
int c_hdel (ARGV_TCL);
int c_help (ARGV_TCL);
int c_hush (ARGV_TCL);
int c_log (ARGV_TCL);
int c_quit (ARGV_TCL);
int c_rand (ARGV_TCL);
int c_replay (ARGV_TCL);
int c_shell (ARGV_TCL);
int c_time (ARGV_TCL);
int c_usage (ARGV_TCL);
int c_version (ARGV_TCL);
int oset (ARGV_TCL);
int ounset (ARGV_TCL);
int s_admin (ARGV_TCL);
int s_beep (ARGV_TCL);
int s_chpw (ARGV_TCL);
int s_group (ARGV_TCL);
int s_motd (ARGV_TCL);
int s_open (ARGV_TCL);
int s_personal (ARGV_TCL);
int s_send (ARGV_TCL);
int s_user (ARGV_TCL);
int s_version (ARGV_TCL);
int s_who (ARGV_TCL);

char *catargs(char **argv);
int cimatch(char *s1, char *s2);
void clearargs(int argc, char *argv[]);
void csendopen(char *txt);
int deleteusage(char *name);
void filtergroupname(char *txt);
void filternickname(char *txt);
void filtertext(char *s);
int fnexit(void);
int fnterm(void);
void getwinsize(void);
void gunlink(USAGE *u, USAGE **first, USAGE **last);
int helpme(char *subject);
int histcount(void);
int hushadd(Tcl_Interp *interp, char *nick);
int hushdelete(Tcl_Interp *interp, char *nick);
int inserttab(void);
int ishushed(char *nick);
void lcaseit(char *s);
void listusage(char *name);
int listvars(char *name);
int logging(void);
int main(int argc, char **argv);
void mbreakprint(int per, char *from, char *s);
void mprint(int per, char *from, char *s);
int numeric(char *txt);
void quoteify(char *a, char *b);
void sendcmd(char *cmd, char *args);
void sendlogin(char *id, char *nick, char *group, char *command, char *passwd);
void sendpong(void);
int setvar(Tcl_Interp *interp, int type, char *address, char *s);
int specialchar(register int c, char *s, char *prompt, int expand);
int split(char *s);
int startsessionlog(Tcl_Interp *interp, char *path);
#ifdef NEED_STRCASECMP
int strcasecmp(char *s1, char *s2); 
int strncasecmp(char *s1, char *s2, register int n); 
#endif
char *strcasestr(const char *s1, const char *s2);
void strlinkafter(STRLIST *s, STRLIST *i, STRLIST **head, STRLIST **tail);
void strlinkalpha(STRLIST *s, STRLIST **head, STRLIST **tail, int caseindep);
void strlinkbefore(STRLIST *s, STRLIST *i, STRLIST **head, STRLIST **tail);
void strlinkhead(STRLIST *s, STRLIST **head, STRLIST **tail);
void strlinktail(STRLIST *s, STRLIST **head, STRLIST **tail);
void strunlink(STRLIST *s, STRLIST **head, STRLIST **tail);
void trapsignals(void);
void uline(USAGE *u);
void usagelinkafter(USAGE *u, USAGE *i, USAGE **first, USAGE **last);
void usagelinkbefore(USAGE *u, USAGE *i, USAGE **first, USAGE **last);
void usagelinkhead(USAGE *u, USAGE **first, USAGE **last);
void usagelinkin(USAGE *u);
void usagelinktail(USAGE *u, USAGE **first, USAGE **last);
int wordcasecmp(char *s1, char *s2);
int wordcmp(char *s1, char *s2);
void c_didpoll(void);
void c_lostcon(void);
void c_packet(char *pkt);
void c_userchar(void);
void read_from_server(void);

/* c_color.c */
char* printcolor (int primary, int fallback);

/* c_log.c */
void closesessionlog (void);

/* c_replay.c */
void bufferadd (char* text);

/* c_time.c */
char *gettimestamp(void);

/* cute.c */
void saysomething (void);

/* dispatch.c */
void dispatch (char* pkt);

/* getc.c */
int getc_or_dispatch (FILE* fp);

/* getline.c */
void eat_server (void);
char *getline (char *prompt);

/* history.c */
void histclear (void);
void histdel (char* name);
void histput (char* nick);

/* main.c */
void icbexit (void);

/* msgs.c */
void beep (char* pkt);
void cmdoutmsg (char* pkt);
void copenmsg (char* pkt);
void cpersonalmsg (char* pkt);
void errormsg (char* pkt);
void exitmsg (void);
void importantmsg (char* pkt);
void loginokmsg (char* pkt);
void protomsg (char* pkt);
void statusmsg (char* pkt);
void grab_url (char* msg);

/* oset.c */
void set_restricted (void);

/* parse.c */
void parse (char* s);

/* print.c */
void putl (char* string, int flags);
void pauseprompt (char* prompt, int erase, int ch, int unget, char* except);

/* readlineint.c */
void readlineinit (void);

/* s_person.c */
void sendpersonal (char* nick, char* text, int echoflag);

/* s_register.c */
int s_register (ARGV_TCL);
void regnick (char* password);

/* s_send.c */
void send_default (char* text, int echoflag);

/* send.c */
void send_command (char* cmd, char* arg);

/* server.c */
int connect_to_server(int use_nick, char* name, int port, char* bind_host);
void listservers(void);

/* strccmp.c */
int ccasecmp (int c1, int c2);

/* strings.c */
char* basename (char* path);
char** msplit (char* string, int soft, int hard);
void msplit_free (char** frags);

/* tcl.c */
void tcl_connected (void);
void tcl_init (void);
void tcl_restrict (void);

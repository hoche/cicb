/* $Id: protocol.h,v 1.4 2001/07/22 21:38:48 alaric Exp $ */

#include "config.h"

#define PACKET_BUF_SIZE 257

/* limits */

#define MAX_FIELDS	20	/* fields in a packet */
#define MAX_GROUPLEN	8	/* chars in a group name */
#define MAX_GROUPUSERS	25	/* people in the same group */
#define MAX_IDLEN	12	/* chars in a login id */
#define MAX_INPUTSTR	(250 - MAX_NICKLEN - MAX_NICKLEN - 6) /* input line */
#define MAX_INVITES	10	/* invitations in a group */
#define MAX_NICKLEN	12	/* chars in a username */
#define MAX_NODELEN	64	/* chars in a node name */
#define MAX_NOPONG	600	/* seconds a client may not PONG a PING */
#define MAX_PASSWDLEN	12	/* chars in a user password */
#define MAX_REALLEN	64	/* chars in a real life name */
#define MAX_TOPICLEN	32	/* chars in a group topic */
#define MAX_IDLE	3600	/* maximum idle seconds before disconnect */

#define IDLE_POLLWAIT	30000	/* polltimeout (msecs) when users = 0 */
#define BUSY_POLLWAIT	10000	/* polltimeout (msecs) when users >= 1 */
#define POLL_DELAY	1000	/* min wait through each poll */

/* message types */
/* note: changing these necessitates upgrading the protocol number */

#define M_LOGIN		'a'	/* login packet */
#define M_LOGINOK	'a'	/* login packet */
#define M_OPEN		'b'	/* open msg to group */
#define M_PERSONAL	'c'	/* personal msg */
#define M_STATUS	'd'	/* status update message */
#define M_ERROR		'e'	/* error message */
#define M_IMPORTANT	'f'	/* special important announcement */
#define M_EXIT		'g'	/* tell other side to exit */
#define M_COMMAND	'h'	/* send a command from user */
#define M_CMDOUT	'i'	/* output from a command */
#define M_PROTO		'j'	/* protocol version information */
#define M_BEEP		'k'	/* beeps */
#define M_PING		'l'	/* ping packet */
#define M_PONG		'm'	/* return for ping packet */

/* These are new definitions for the protocol */
#define S_PERSONAL	M_PERSONAL

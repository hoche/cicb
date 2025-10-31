/* $Id: dispatch.c,v 1.8 2003/07/07 01:42:15 felixlee Exp $ */

/* handle a packet from the server */

#include "icb.h"

void dispatch(char *pkt) {
    switch (*pkt) {

    case M_PING:
        sendpong();
        break;

    case M_BEEP:
        beep(++pkt);
        break;

    case M_PROTO:
        protomsg(++pkt);
        break;

    case M_LOGINOK:
        loginokmsg(++pkt);
        break;

    case M_OPEN:
        copenmsg(++pkt);
        break;

    case M_STATUS:
        statusmsg(++pkt);
        break;

    case M_ERROR:
        errormsg(++pkt);
        break;

    case M_IMPORTANT:
        importantmsg(++pkt);
        break;

    case M_EXIT:
        exitmsg();
        break;

    case M_CMDOUT:
        cmdoutmsg(++pkt);
        break;

    case M_PERSONAL:
        cpersonalmsg(++pkt);
        break;

    default:
        snprintf(
            mbuf, MESSAGE_BUF_SIZE, "%s[=Error=] Invalid packet type \"%c\"%s",
            printcolor(ColERROR, ColSANE), *pkt, printcolor(ColSANE, ColSANE));
        putl(mbuf, PL_ALL);
    }
}

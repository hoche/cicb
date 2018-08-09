/* $Id: strccmp.c,v 1.10 2003/07/07 01:42:16 felixlee Exp $ */

/*
 * Except for strcasestr, which is released under the GNU license,
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "config.h"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strcasecmp.c	5.6 (Berkeley) 6/27/88";
#endif                          /* LIBC_SCCS and not lint */

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static unsigned char charmap[] = {
    '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
    '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
    '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
    '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
    '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
    '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
    '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
    '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
    '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
    '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
    '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
    '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
    '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
    '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
    '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
    '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
    '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
    '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
    '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
    '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
    '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
    '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

#ifdef NEED_STRCASECMP

int
strcasecmp(char *s1, char *s2)
{
    register unsigned char *cm = charmap, *us1 = (char *)s1, *us2 = (char *)s2;

    while (cm[*us1] == cm[*us2++])
        if (*us1++ == '\0')
            return (0);
    return (cm[*us1] - cm[*--us2]);
}

int
strncasecmp(char *s1, char *s2, int n)
{
    register unsigned char *cm = charmap, *us1 = (char *)s1, *us2 = (char *)s2;

    while (--n >= 0 && cm[*us1] == cm[*us2++])
        if (*us1++ == '\0')
            return (0);
    return (n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}

#endif

int
ccasecmp(int c1, int c2)
{
    register unsigned char *cm = charmap;
    return (cm[c1] == cm[c2]);
}

/*
 * Originally written by Stephen R. van den Berg, and released under the GNU
 * licence. See original comments below. 
 *
 * Adapted for strcasestr and commented by Michel Hoche-Mong, hoche@grok.com. 
 * Other than the blurb below, there were no comments in this code before I 
 * got my hands on it.
 */

/* 
   Copyright (C) 1994, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de	*/

typedef unsigned chartype;

char *
strcasestr(const char* phaystack, const char* pneedle)
{
    register unsigned char *cm = charmap;
    register const unsigned char *haystack, *needle;
    register chartype b, c;

    haystack = (const unsigned char *)phaystack;
    needle = (const unsigned char *)pneedle;

    b = *needle;
    if (b != '\0') {
        /* back up to one before the string to set up */
        /* our do/while loop. possible ANSI violation */
        haystack--;

        /* skip over any chars in the haystack that */
        /* don't match the first char of the needle */
        do {
            c = *++haystack;
            if (c == '\0')
                return 0;
        }
        while (cm[c] != cm[b]);

        /* ok, we found a spot where the haystack might match the needle */
        /* needle. if the needle's only one char long, we're done. otherwise */
        /* look at the next char of the needle and jump into the middle of */
        /* our loop. */
        c = *++needle;
        if (c == '\0')
            goto foundneedle;
        ++needle;
        goto jin;

        for (;;) {
            register chartype a;
            register const unsigned char *rhaystack, *rneedle;

            /* walk forward in haystack until we find a character that */
            /* matches the first one in needle. */
            do {
                a = *++haystack;
                if (a == '\0')
                    return 0;

                if (cm[a] == cm[b])
                    break;

                a = *++haystack;
                if (a == '\0')
                    return 0;
shloop:        ;

            }
            while (cm[a] != cm[b]);

jin:
            /* look at the next char of the haystack. if we're here, we've */
            /* got a character in the haystack that matches the starting */
            /* character of the needle. we're now going to examine the second */
            /* char of our possible match. */
            a = *++haystack;
            if (a == '\0')
                return 0;

            /* if the characters are different, this can't be the needle, so */
            /* go to the section above which walks a little further along */
            /* haystack and we'll start looking for the needle all over.  */
            if (cm[a] != cm[c])
                goto shloop;

            /* set rhaystack to the start of the possible match section, */
            /* and rneedle to the start of needle. */
            rhaystack = haystack-- + 1;
            rneedle = needle;
            a = *rneedle;

            /* this little loop tests two characters in the sequence */
            /* at a time for more loop efficiency. Could probably squeeze a */
            /* teensy bit more out of it by testing even more characters at */
            /* once, say five or so, but this is the way it was originally. */

            /* note from the comment-writer: not sure why we test again here */
            /* since we already know the first character matches. */
            if (cm[*rhaystack] == cm[a]) {
                do {
                    if (a == '\0')
                        goto foundneedle;

                    /* move rhaystack and rneedle up one and test the chars */
                    ++rhaystack;
                    a = *++needle;
                    if (cm[*rhaystack] != cm[a])
                        break;
                    if (a == '\0')
                        goto foundneedle;

                    /* still ok, try another pair of chars */
                    ++rhaystack;
                    a = *++needle;

                } while (cm[*rhaystack] == cm[a]);
            }

            /* nope, haystack doesn't match the needle. reset the needle and */
            /* go to the part of the loop that walks forward in haystack. */
            needle = rneedle;   /* took the register-poor approach */

            if (a == '\0')
                break;
        }
    }

foundneedle:
    return (char *)haystack;
}

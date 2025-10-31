/* $Id: strings.c,v 1.20 2003/07/07 01:42:16 felixlee Exp $ */

#include "icb.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>

extern char *charmap;

#define OKGROUPCHARS "-.!'$+,/?_"
#define OKNICKCHARS "-.!'$+,/?_"

/* replace illegal characters in a nickname */

void filternickname(char *txt) {
    for (; *txt != '\0'; txt++) {
        if ((*txt >= 'A' && *txt <= 'Z') || (*txt >= 'a' && *txt <= 'z') ||
            (*txt >= '0' && *txt <= '9'))
            continue;
        if (! strchr(OKNICKCHARS, *txt)) {
            if (*txt == ' ')
                *txt = '_';
            else
                *txt = '?';
        }
    }
}

/* replace illegal characters in a regular line of text */

void filtertext(char *s) {
    for (; *s != '\0'; s++)
        if (! (*s >= ' ' && *s < '\177'))
            *s = '_';
}

/* replace illegal characters from a groupname */

void filtergroupname(char *txt) {
    for (; *txt != '\0'; txt++) {
        if ((*txt >= 'A' && *txt <= 'Z') || (*txt >= 'a' && *txt <= 'z') ||
            (*txt >= '0' && *txt <= '9'))
            continue;
        if (! strchr(OKGROUPCHARS, *txt)) {
            if (*txt == ' ')
                *txt = '_';
            else
                *txt = '?';
        }
    }
}

/* Safe strncpy replacement that always null-terminates.
 * Copies at most (dest_size - 1) characters from src to dest,
 * always null-terminating the result.
 * Returns 0 on success, -1 if truncation occurred.
 */
int safe_strncpy(char *dest, const char *src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;
    }

    if (dest_size == 1) {
        dest[0] = '\0';
        return (*src != '\0') ? -1 : 0;
    }

    size_t src_len = strlen(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : (dest_size - 1);

    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';

    return (src_len >= dest_size) ? -1 : 0;
}

/* Safe strncat replacement with bounds checking.
 * Appends at most (dest_size - dest_len - 1) characters from src to dest,
 * always null-terminating the result.
 * Returns 0 on success, -1 if truncation occurred.
 */
int safe_strncat(char *dest, const char *src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;
    }

    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size) {
        return -1; /* Already at or past buffer size */
    }

    size_t available = dest_size - dest_len - 1;
    size_t src_len = strlen(src);
    size_t copy_len = (src_len < available) ? src_len : available;

    memcpy(dest + dest_len, src, copy_len);
    dest[dest_len + copy_len] = '\0';

    return (src_len > available) ? -1 : 0;
}

/* Safe integer parsing function.
 * Converts string to int with full validation.
 * Returns 0 on success, -1 on error.
 * On success, *result contains the parsed integer.
 * Validates that entire string is consumed and value is in int range.
 */
int safe_atoi(const char *str, int *result) {
    char *endptr;
    long val;

    if (str == NULL || *str == '\0') {
        return -1;
    }

    errno = 0;
    val = strtol(str, &endptr, 10);

    /* Check for conversion errors */
    if (endptr == str) {
        /* No digits found */
        return -1;
    }

    /* Check if entire string was consumed (allow trailing whitespace) */
    while (*endptr != '\0' && isspace((unsigned char) *endptr)) {
        endptr++;
    }
    if (*endptr != '\0') {
        /* Extra characters after number */
        return -1;
    }

    /* Check for overflow/underflow */
    if ((val == LONG_MAX || val == LONG_MIN) && errno == ERANGE) {
        return -1;
    }

    /* Check if value fits in int */
    if (val > INT_MAX || val < INT_MIN) {
        return -1;
    }

    *result = (int) val;
    return 0;
}

/* return 1 if a string is a number */
/* else return 0 */

char *findspace(char *s) {
    /* find EOS or whitespace */
    while (*s != '\0' && *s != ' ' && *s != '\t')
        s++;

    if (*s == '\0')
        /* oops found no whitespace */
        return (0);
    else {
        /* point to char *after* whitespace */
        *s = '\0';
        return (++s);
    }
}

/* convert a string to lower case */
void lcaseit(char *s) {
    for (; *s; s++)
        if (*s >= 'A' && *s <= 'Z')
            *s |= 040;
}

/* put a string in quotes */
/* puts backslashes before all special chars appearing in the string */
/* doesn't interfere with already backslashed chars */

char *special = "{}[]\";$\\";

/* Safe version with bounds checking */
void quoteify(char *a, char *b) {
    /* Legacy call - assume buffer is large enough */
    /* For new code, use quoteify_safe() with explicit size */
    if (a == NULL || b == NULL) {
        if (b)
            b[0] = '\0';
        return;
    }

    while (*a != '\0') {
        if (strchr(special, *a)) {
            *b++ = '\\';
            *b++ = *a;
        } else
            *b++ = *a;
        a++;
    }
    *b = '\0';
}

/* Safe version with bounds checking */
void quoteify_safe(char *a, char *b, size_t b_size) {
    if (a == NULL || b == NULL || b_size == 0) {
        if (b && b_size > 0)
            b[0] = '\0';
        return;
    }

    size_t remaining = b_size - 1; /* Reserve space for null terminator */

    while (*a != '\0' && remaining > 0) {
        if (strchr(special, *a)) {
            if (remaining < 2) {
                /* Not enough space for escape sequence */
                break;
            }
            *b++ = '\\';
            *b++ = *a;
            remaining -= 2;
        } else {
            *b++ = *a;
            remaining--;
        }
        a++;
    }
    *b = '\0';
}

static char *catargsbuf = NULL;

char *catargs(char **argv) {
    char *s, **ap, *p;
    int len = 0;

    for (ap = argv; *ap; ++ap) {
        len += strlen(*ap) + 1;
    }

    if (catargsbuf)
        free(catargsbuf);

    if ((catargsbuf = (char *) malloc(len + 1)) == NULL) {
        fprintf(stderr, "Out of memory in catargs().\n");
        icbexit();
    }

    p = catargsbuf;
    while (*argv != NULL) {
        s = *argv;
        while (*s)
            *p++ = *s++;
        if (*(argv + 1) != NULL)
            *p++ = ' ';
        argv++;
    }
    *p = '\0';
    return catargsbuf;
}

/* Print a line, splitting it (at a space) if necessary to make it fit
 * the screenwidth.
 *
 * if per = 0, it's an open message
 * if per = 1, it's a personal message and we need to do some additional
 *    padding.
 *
 * from is the nick from which the message came
 * s is the message itself, NULL terminated
 */
void mbreakprint(int per, char *from, char *s) {
    int width;
    int padlen;
    char **pieces;
    char **p;

    if (per) {
        padlen = 2 + 3; /* pad nick with "<*" and "*> " */
    } else {
        padlen = 1 + 2; /* pad nick with "<" and "> " */
    }
    if (gv.timestamp) {
        padlen += 8;
    }

    /* if gv.pagewidth < 0, it means set it was set dynamically from the
     *    window size, usually by handling SIGWINCH.
     * if gv.pagewidth == 0, it means the user doesn't ever want it to linebreak
     * if pagewidth > 0, it means the user wanted a fixed width.
     */
    width = gv.pagewidth;
    if (width < 0) {
        width = -width;
    }
    width -= padlen + strlen(from);

    /* split and print it */

    /* note: at this point width can be <0 (if it was set really small or
     * 0 to begin with */
    pieces = msplit(s, width, 0);

    for (p = pieces; *p != NULL; ++p) {
        mprint(per, from, *p);
    }

    msplit_free(pieces);
}

void mprint(int per, char *from, char *s) {

    /*	In here, we want to test for the existance of from in the tcl
 *	variable personalhilite, and if they exist, printcolor(ColPERSHILITE)
 *	instead.   This means, we should probably have this variable linked
 *	at all times. 
 */

    if (per)
        if ((personalhilite != NULL) && (strlen((char *) personalhilite)) &&
            (strcasestr((const char *) personalhilite, (const char *) from)))
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%s<*%s%s%s*>%s %s%s%s",
                     printcolor(ColPERSFROMHILITE, ColPERSFROM),
                     printcolor(ColPERSFROMHILITE, ColPERSFROM), from,
                     printcolor(ColPERSFROMHILITE, ColPERSFROM),
                     printcolor(ColSANE, ColSANE),
                     printcolor(ColPERSHILITE, ColPERSONAL), s,
                     printcolor(ColSANE, ColSANE));
        else
            snprintf(mbuf, MESSAGE_BUF_SIZE, "%s<*%s%s%s*>%s %s%s%s",
                     printcolor(ColPBRKT, ColPERSFROM),
                     printcolor(ColPERSFROM, ColPERSFROM), from,
                     printcolor(ColPBRKT, ColPERSFROM),
                     printcolor(ColSANE, ColSANE),
                     printcolor(ColPERSONAL, ColPERSONAL), s,
                     printcolor(ColSANE, ColSANE));
    else
        snprintf(mbuf, MESSAGE_BUF_SIZE, "%s<%s%s%s>%s %s%s%s",
                 printcolor(ColABRKT, ColNICKNAME),
                 printcolor(ColNICKNAME, ColNICKNAME), from,
                 printcolor(ColABRKT, ColNICKNAME),
                 printcolor(ColSANE, ColSANE), printcolor(ColNORMAL, ColNORMAL),
                 s, printcolor(ColSANE, ColSANE));
    putl(mbuf, PL_ALL);
}

int wordcmp(char *s1, char *s2) {
    while (*s1 == *s2++)
        if (*s1 == '\0' || *s1 == ' ' || *s1++ == '\t')
            return (0);
    if (*s1 == ' ' && *(s2 - 1) == '\0')
        return (0);
    return (*s1 - *--s2);
}

char *getword(char *s) {
    static char word[64];
    char *w = word;
    while (*s != ' ' && *s != '\t' && *s != '\0' && ((w - word) < 64))
        *w++ = *s++;
    *w = '\0';
    return (word);
}

/* Read a line containing zero or more colons. Split the string into */
/* an array of strings, and return the number of fields found. */

char *fields[MAX_FIELDS];

int split(char *s) {
    char *p = s;
    int i = 0;

    fields[i] = s;
    for (;;) {

        i++;

        /* find delim or EOS */
        while (*p != '\001' && *p != '\0')
            p++;

        if (*p == '\001') {
            /* got delim */
            *p = '\0';
            fields[i] = ++p;
        } else
            return (i);
    }
}

/* malloc a new string copy of the N chars starting at S,
   possibly adding a '-' if HYPHEN is nonzero */

static char *msplit_dup(char *s, int n, int hyphen) {
    char *dup = malloc(n + (hyphen != 0) + 1);
    if (dup != NULL) {
        memcpy(dup, s, n);
        if (hyphen) {
            dup[n++] = '-';
        }
        dup[n] = '\0';
    }
    return dup;
}

/* Split STRING for msplit().
 *
 *  Returns the number of fragments.  Returns -1 on malloc error.
 *
 *  If FRAGS is non-null, then it gets filled with pointers to malloc'ed
 *  copies of the fragments.  FRAGS must be large enough.
 *
 *  If soft <= 0 and hard <= 0, don't split, just copy everything to the first
 *      frag
 *  If soft <= 0 and hard > 0, split at hard point
 *  If soft > 0 and hard <= 0, try to split at or prior to the soft point, but
 *     if there's no convenient space, split at the first space after the soft
 *     point.
 *  If soft > 0 and hard > 0, try to split at or prior to the soft point, but
 *     if there's no convenient space, force a split at the hard point.
 */

static int msplit_internal(char *string, int soft, int hard, char **frags) {
    int n_frags;
    int string_len;

    if (string == NULL) {
        return 0;
    }

    if (soft <= 0) {
        soft = hard;
    }

    n_frags = 0;
    string_len = strlen(string);
    if (soft > 0) {
        while (soft < string_len) {
            char *p = string + soft;

            /* find last space prior to our current position */
            while (string < p && ! isspace(*p)) {
                --p;
            }

            /* If no spaces, or space is too close to the beginning... */
            if (p < string + soft / 2) {

                /* find next space */
                p = string + soft;
                while (*p != '\0' && ! isspace(*p)) {
                    ++p;
                }

                /* if the hard break is set and this fragment is bigger than
                 * that, force truncation even if it's in the middle of a word */
                if (hard > 0 && string + hard < p) {
                    p = string + hard;
                }
            }

            /* Copy it out */
            if (frags != NULL) {
                char *frag = msplit_dup(string, p - string, 0);
                if (frag == NULL)
                    return -1;
                frags[n_frags] = frag;
            }
            ++n_frags;

            /* Skip over the space */
            if (*p != '\0' && isspace(*p)) {
                ++p;
            }

            string_len -= p - string;
            string = p;
        }
    }

    /* last fragment */
    if (*string != '\0') {
        if (frags != NULL) {
            char *frag = msplit_dup(string, string_len, 0);
            if (frag == NULL)
                return -1;
            frags[n_frags] = frag;
        }
        ++n_frags;
    }
    return n_frags;
}

/* Split STRING, usually at space chars, into fragments usually <=SOFT
 * chars.  STRING is not modified.
 *
 * Returns a malloc'ed array of malloc'ed strings.  Returns NULL on malloc
 * error.
 *
 * Any returned array should be destroyed with msplit_free().
 *
 * A fragment can be >SOFT chars if there are no spaces in the fragment, or
 * if splitting at a space would create a fragment <SOFT/2 chars.
 *
 * If HARD is non-zero, then all fragments are forced to be <=HARD chars,
 * which may require splitting at a non-space char.
 *
 * If the split point is a space, the space isn't included in the fragments.
 */
char **msplit(char *string, int soft, int hard) {
    char **frags;
    int n_frags;
    int k;

    /* Find out how many fragments we're going to make. */
    n_frags = msplit_internal(string, soft, hard, NULL);
    if (n_frags < 0) {
        return NULL;
    }

    /* Allocate the array. */
    frags = (char **) malloc(sizeof(char *) * (n_frags + 1));
    if (frags == NULL) {
        return NULL;
    }

    /* Zero the array. */
    for (k = 0; k < n_frags + 1; ++k) {
        frags[k] = NULL;
    }

    /* Do the split. */
    n_frags = msplit_internal(string, soft, hard, frags);
    if (n_frags < 0) {
        msplit_free(frags);
        return NULL;
    }

    return frags;
}

/* Calls free() on a STRINGS array as returned by msplit(). */

void msplit_free(char **strings) {
    if (strings != NULL) {
        char **p = strings;
        for (; *p != NULL; ++p) {
            free(*p);
        }
        free(strings);
    }
}

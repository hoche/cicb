/* $Id: c_time.c,v 1.8 2003/07/07 01:42:15 felixlee Exp $ */

#include "icb.h"

/* qtime.c	Displays time in real English, also chimes
   02/90	Sean Casey 	Modified for use in forum client.
   09/89	Ade Lovett	Complete rewrite
   04/86	Mark Dapoz	Converted to C for UNIX
   12/79-12/82	Mike Cowlishaw */

static char *eng_1min[] = {
    "",
    "just after ",
    "a little after ",
    "nearly ",
    "just about "
};

static char *eng_5min[] = {
    "",
    "five past ",
    "ten past ",
    "a quarter past ",
    "twenty past ",
    "twenty-five past ",
    "half past ",
    "twenty-five to ",
    "twenty to ",
    "a quarter to ",
    "ten to ",
    "five to ",
    ""
};

static char *eng_hour[] = {
    "",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
};

static void
put_cute_time(struct tm *now)
{
    char buf[256];
    int m;
    int h;

    m = now->tm_min + (29 < now->tm_sec);
    h = now->tm_hour + (32 < m);

    /* make sure the values are in range. */
    m = m % 60;
    if (m < 0) {
        m += 60;
    }
    h = h % 24;
    if (h < 0) {
        h += 24;
    }

    strcpy(buf, "It's ");
    strcat(buf, eng_1min[m % 5]);
    strcat(buf, eng_5min[m / 5 + (2 < m % 5)]);

    if (h == 0) {
        strcat(buf, "midnight.");

    } else if (h == 12) {
        strcat(buf, "noon.");

    } else {
        strcat(buf, eng_hour[h % 12]);
        if (m == 0) {
            strcat(buf, " o'clock");
        }
        if (20 <= h || h < 4) {
            strcat(buf, " at night");
        } else if (4 <= h && h < 12) {
            strcat(buf, " in the morning");
        } else if (12 <= h && h < 17) {
            strcat(buf, " in the afternoon");
        } else if (17 <= h && h < 20) {
            strcat(buf, " in the evening");
        }
        strcat(buf, ".");
    }

    if (m != 0 && m % 15 == 0) {
        strcat(buf, "  *ding*");
    }

    putl(buf, PL_SCR);

    if (m == 0) {
        strcpy(buf, "(");
        if (h == 0) {
            h = 12;
        } else if (12 < h) {
            h -= 12;
        }
        for (; 1 < h; --h) {
            strcat(buf, "Bong, ");
        }
        strcat(buf, "Bong!)");
        putl(buf, PL_SCR);
    }
}

static void
put_time(struct tm *now, char *format)
{
    char buf[256];
    int n = strftime(buf, sizeof buf, format, now);

    /* old glibc is bugged and returns nonzero if there's no room. */
    if (0 < n && n < sizeof buf) {
        putl(buf, PL_SL);
    } else {
        putl("Time string too long.", PL_SCR);
    }
}

int
c_time(ARGV_TCL)
{
    time_t t;
    struct tm *now;
    char *style = gv.timedisplay;

    if (argc == 1) {
        t = time(NULL);
    } else if (argc == 2) {
        t = atoi(argv[1]);
    } else {
        RETURN_ERR_ARGC("?timeval?");
    }

    now = localtime(&t);

    if (strcmp(style, "cute") == 0) {
        put_cute_time(now);

    } else if (strcmp(style, "normal") == 0) {
        put_time(now, "The time is %I:%M%p.");

    } else if (strcmp(style, "military") == 0) {
        put_time(now, "The time is %H:%M.");

    } else {
        put_time(now, style);
    }

    return TCL_OK;
}

/* returns a pointer to a statically allocated buffer containing an 
 * updated timestamp (including a trailing space).
 */
char *
gettimestamp(void)
{
    static char tsbuf[128];
    time_t t;
    struct tm *now;

    t = time(NULL);
    now = localtime(&t);

    sprintf(tsbuf,
            "%s[%02d:%02d]%s ",
            printcolor(ColTIMESTAMP, ColSANE),
            now->tm_hour, now->tm_min, printcolor(ColSANE, ColSANE));
    /*COLOR*/ return tsbuf;
}

/* Minimal implementation of safe string functions for testing */
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/* Safe strncpy replacement that always null-terminates. */
int
safe_strncpy(char *dest, const char *src, size_t dest_size)
{
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

/* Safe strncat replacement with bounds checking. */
int
safe_strncat(char *dest, const char *src, size_t dest_size)
{
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;
    }
    
    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size) {
        return -1;  /* Already at or past buffer size */
    }
    
    size_t available = dest_size - dest_len - 1;
    size_t src_len = strlen(src);
    size_t copy_len = (src_len < available) ? src_len : available;
    
    memcpy(dest + dest_len, src, copy_len);
    dest[dest_len + copy_len] = '\0';
    
    return (src_len > available) ? -1 : 0;
}

/* Safe integer parsing function. */
int
safe_atoi(const char *str, int *result)
{
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
    while (*endptr != '\0' && isspace((unsigned char)*endptr)) {
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

    *result = (int)val;
    return 0;
}

/* Quoteify functions */
static char *special = "{}[]\";$\\";

void
quoteify(char *a, char *b)
{
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

void
quoteify_safe(char *a, char *b, size_t b_size)
{
    if (a == NULL || b == NULL || b_size == 0) {
        if (b && b_size > 0)
            b[0] = '\0';
        return;
    }
    
    size_t remaining = b_size - 1;  /* Reserve space for null terminator */
    
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

/* Convert string to lowercase */
void
lcaseit(char *s)
{
    for (; *s; s++)
        if (*s >= 'A' && *s <= 'Z')
            *s |= 040;
}

/* Find space or tab in string */
char *
findspace(char *s)
{
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


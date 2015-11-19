/* parsing.c
 * See https://github.com/pepaslabs/nbdkit-chunks-plugin
 * Copyright (C) 2015 Jason Pepas.
 * Released under the terms of the MIT license.
 * See https://opensource.org/licenses/MIT
 */

#include "parsing.h"

#include <limits.h> // INT_MAX, etc.
#include <string.h> // strncasecmp(), etc.
#include <stdlib.h> // strtoull(), etc.
#include <errno.h> // errno, etc.


int copy_numeric(const char *in, size_t in_size, char *out, size_t out_size)
{
    if (in_size == 0 || out_size == 0) return 0;

    int i = 0;
    bool stop = false;
    while (i < in_size && i < out_size && i < INT_MAX && !stop)
    {
        if (i == in_size-1 || i == out_size-1)
        {
            stop = true;
            continue;
        }

        char ch = in[i];
        switch (ch)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                out[i] = ch;
                i++;
                continue;

            default:
                stop = true;
                continue;
        }
    }
    out[i] = '\0';

    return i;
}


int parse_uint64t_str_strict(const char *in, size_t in_size, uint64_t *out)
{
    if (in_size <= 0+1)
    {
        return ERROR_parse_uint64t_str_strict_BAD_in_size_ZERO;
    }

    char int_buf[sizeof("18446744073709551615")];

    if (in_size > sizeof(int_buf))
    {
        return ERROR_parse_uint64t_str_strict_BAD_in_size_TOO_LARGE;
    }

    int chars_copied = copy_numeric(in, in_size, int_buf, sizeof(int_buf));
    if (chars_copied != in_size-1)
    {
        return ERROR_parse_uint64t_str_strict_NOT_ALL_CHARS_PARSED;
    }

    int saved_errno = errno;
    errno = 0;

    int base = 10;
    unsigned long long int size = strtoull(int_buf, NULL, base);
    if (errno)
    {
        return ERROR_parse_uint64t_str_strict_strtoull_FAILED;
    }
    errno = saved_errno;

    *out = size;

    return RETURN_SUCCESS;
}


int parse_suffix(const char *in, size_t in_size, suffix_t *out)
{
    if (in_size == 0+1)
    {
        *out = 0;
    }
    else if (in_size == 1+1)
    {
        if (strncasecmp(in, "k", 1) == 0) *out = K;
        else if (strncasecmp(in, "m", 1) == 0) *out = M;
        else if (strncasecmp(in, "g", 1) == 0) *out = G;
        else if (strncasecmp(in, "t", 1) == 0) *out = T;
        else return ERROR_parse_suffix_UNRECOGNIZED_SUFFIX;
    }
    else if (in_size == 2+1)
    {
        if (strncasecmp(in, "kb", 2) == 0) *out = KB;
        else if (strncasecmp(in, "mb", 2) == 0) *out = MB;
        else if (strncasecmp(in, "gb", 2) == 0) *out = GB;
        else if (strncasecmp(in, "tb", 2) == 0) *out = TB;
        else return ERROR_parse_suffix_UNRECOGNIZED_SUFFIX;
    }
    else
    {
        return ERROR_parse_suffix_BAD_in_size;
    }

    return RETURN_SUCCESS;
}


uint64_t suffix_to_multiplier(const suffix_t *suffix)
{
    switch (*suffix)
    {
        case KB: return 1000;
        case K:  return 1024;
        case MB: return 1000*1000;
        case M:  return 1024*1024;
        case GB: return 1000*1000*1000;
        case G:  return 1024*1024*1024;
        case TB: return (uint64_t)1000*1000*1000*1000;
        case T:  return (uint64_t)1024*1024*1024*1024;
        default: return 1;
    }
}


int parse_size_str(const char *in, size_t in_size, uint64_t *out)
{
    int retval;

    char int_buf[sizeof("18446744073709551615")];
    int chars_copied = copy_numeric(in, in_size, int_buf, sizeof(int_buf));
    if (chars_copied == 0)
    {
        return ERROR_parse_size_str_NO_LEADING_NUMERIC_CHARS;
    }

    int base = 10;
    unsigned long long int size = strtoull(int_buf, NULL, base);

    char *suffix_in = (char*)in + chars_copied;
    size_t suffix_size = in_size - chars_copied;
    suffix_t suffix;
    retval = parse_suffix(suffix_in, suffix_size, &suffix);
    if (retval < 0) return retval;

    uint64_t multiplier = suffix_to_multiplier(&suffix);

    // thanks to http://stackoverflow.com/a/199363
    bool would_overflow = (multiplier > (UINT64_MAX / size));
    if (would_overflow)
    {
        return ERROR_parse_size_str_WOULD_OVERFLOW_UINT64;
    }

    *out = size * multiplier;

    return RETURN_SUCCESS;
}

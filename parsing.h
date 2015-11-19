/* parsing.h
 * See https://github.com/pepaslabs/nbdkit-chunks-plugin
 * Copyright (C) 2015 Jason Pepas.
 * Released under the terms of the MIT license.
 * See https://opensource.org/licenses/MIT
 */

#ifndef PARSING_H
#define PARSING_H

#include <stddef.h> // size_t, etc.
#include <stdint.h> // uint64_t, etc.
#include <stdbool.h> // bool, etc.

#include "retsucc.h" // RETURN_SUCCESS, etc.


// copy in to out, stopping at the first non-numeric character.
// in_size is the length of 'in' plus the terminating '\0'.
// returns the number of characters copied.
int copy_numeric(const char *in, size_t in_size, char *out, size_t out_size);


// parses a numeric string.
// in_size is the length of 'in' plus the terminating '\0'.
// returns RETURN_SUCCESS (0) on succes.
// returns error (-1) on any non-numeric character.
int parse_uint64t_str_strict(const char *in, size_t in_size, uint64_t *out);

enum {
	ERROR_parse_uint64t_str_strict_BAD_in_size_ZERO = -1,
	ERROR_parse_uint64t_str_strict_BAD_in_size_TOO_LARGE = -2,
    ERROR_parse_uint64t_str_strict_NOT_ALL_CHARS_PARSED = -3,
    ERROR_parse_uint64t_str_strict_strtoull_FAILED = -4
};


typedef enum { K=1, KB, M, MB, G, GB, T, TB } suffix_t;

// translates a suffix string into a suffix_t.
// out is set to a suffix_t value or 0.
// in_size is the length of 'in' plus the terminating '\0'.
// returns RETURN_SUCCESS (0) on succes.
// returns a negative value on error.
int parse_suffix(const char *in, size_t in_size, suffix_t *out);

enum {
    ERROR_parse_suffix_UNRECOGNIZED_SUFFIX = -1,
    ERROR_parse_suffix_BAD_in_size = -2
};


// converts a suffix_t enum into a numeric multiplier, or 1.
uint64_t suffix_to_multiplier(const suffix_t *suffix);


// parses a numeric string, with an optional mulitiplier suffix.
// in_size is the length of 'in' plus the terminating '\0'.
// returns RETURN_SUCCESS (0) on succes.
// returns a negative value on error.
int parse_size_str(const char *in, size_t in_size, uint64_t *out);

enum {
    // can also return ERROR_parse_suffix_UNRECOGNIZED_SUFFIX (-1)
    // can also return ERROR_parse_suffix_BAD_in_size (-2)
    ERROR_parse_size_str_NO_LEADING_NUMERIC_CHARS = -3,
    ERROR_parse_size_str_WOULD_OVERFLOW_UINT64 = -4
};


#endif // PARSING_H

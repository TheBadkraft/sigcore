#ifndef STRINGS_H
#define STRINGS_H


#include "sigcore.h"
#include "sigdebug.h"
#include "collections.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*
 *	Unified utilities for string and string_bulder
 */

/* IString Helpers */
size_t str_get_length(const string);
string str_copy(const string);
string str_concat(const string, const string);
int str_compare(const string, const string);

/* IStringBuilder Helpers */
string_builder sb_new(size_t);
string_builder sb_from_string(string);
size_t sb_get_length(string_builder);
void sb_append(string_builder, string);
string sb_to_string(string_builder sb);
void sb_free(string_builder);
#endif

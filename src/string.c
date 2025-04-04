// src/string.c
/*
 * Implements efficient string utilities.
 */
#include "strings.h"
#include "mem_utils.h"
#include <stdlib.h>

/* Returns the length of a string */
size_t str_get_length(const string str) {
	return str ? strlen(str) : 0;
}
/* Returns a copy of given string */
string str_copy(const string str) {
	size_t len = str_get_length(str);
	string copy = NULL;
	if (len > 0) {
		copy = Mem.alloc(len + 1);
		if (copy) strcpy(copy, str);
	}
	
	return copy;
}
/* Returns a duplcate string */
string str_duplicate(const char* str) {
	string s = strdup(str);
	if (!s) return NULL;
	
	if (!trackMem(s)) {
		free(s);
		return NULL;
	}
	
	return s;
}
/* Returns a concatenated string */
string str_concat(const string str1, const string str2) {
	string result = NULL;
	if (!str1 || !str2) return result;
	
	string_builder sb = sb_from_string(str1);
	if (!sb) return result;
	
	sb_append(sb, str2);
	result = sb_to_string(sb);
	sb_free(sb);
	
	return result;
}
/* Compares two strings */
int str_compare(const string str1, const string str2) {
	if (str1 == str2) return 0;
	if (!str1 || !str2) return 1;
	
	return strcmp(str1, str2);
}
/* Returns a formatted string */
static string formatString (string format, ...) {
	if (!format) return NULL;
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	va_end(args);
	if (len < 0) return NULL;

	string_builder sb = sb_new(len + 1);
	if (!sb) return NULL;
	va_start(args, format);
	vsnprintf((char*)sb->buffer, len + 1, format, args);
	sb->last = (addr)sb->buffer + len - 1;
	va_end(args);

	string result = sb_to_string(sb);
	sb_free(sb);
	
	return result;
}
/* Free string from allocated memory */
static void freeString(string str) {
	if (str) Mem.free(str);
}

const IString String = {
	.length = str_get_length,
	.copy = str_copy,
	.dupe = str_duplicate,
	.concat = str_concat,
	.compare = str_compare,
	.format = formatString,
	.free = freeString,
};

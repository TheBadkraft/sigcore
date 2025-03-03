// src/string_builder.c
/*
 * Implements an efficient string builder for sigcore, inspired by .NET StringBuilder.
 * Provides the IStringBuilder interface for concatenating strings without repeated allocations.
 * Uses a single char buffer with last as the address of the last used character and end as the address
 * of the null terminator (buffer + capacity). Allocates capacity + 1 bytes, resizes by doubling when needed,
 * and integrates with Mem and Collections for memory management and buffer operations. Ideal for text
 * construction in sigcore applications like rendering or logging.
 */
#include "sigcore.h"
#include "collections.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static size_t getLength(string_builder sb);
static size_t getCapacity(string_builder sb);

/* Initializes a string builder with the given capacity, allocating buffer + 1 for null */
static string_builder newStringBuilder(size_t capacity) {
	if (capacity == 0) capacity = 16;
	string_builder sb = Mem.alloc(sizeof(struct string_builder_s));
	if (!sb) return NULL;
	
	sb->buffer = Mem.alloc(capacity + 1);
	if (!sb->buffer) {
		Mem.free(sb);
		return NULL;
	}
	sb->last = (addr)sb->buffer - 1;
	sb->end = (addr)sb->buffer + capacity;
	sb->buffer[0] = '\0';
	return sb;
}
/* Appends a plain string to the buffer, resizing if necessary */
static void append(string_builder sb, string str) {
	if (!sb || !str) return;
	size_t len = strlen(str);
	size_t current_len = sb->last - (addr)sb->buffer + 1;
	if (sb->last + len + 1 >= sb->end) {
		size_t old_capacity = sb->end - (addr)sb->buffer;
		size_t new_capacity = old_capacity ? old_capacity * 2 : 16;
		while (new_capacity < current_len + len) new_capacity *= 2;
		char* new_buffer = Mem.alloc(new_capacity + 1);
		if (!new_buffer) return;
		Collections.copyTo((addr*)sb->buffer, (addr*)new_buffer, sb->last + 1);
		Collections.clear((addr*)(new_buffer + current_len), (addr)(new_buffer + new_capacity + 1));
		Mem.free(sb->buffer);
		sb->last = (addr)new_buffer + current_len - 1;
		sb->buffer = new_buffer;
		sb->end = (addr)new_buffer + new_capacity;
	}
	sb->last++;
	memcpy((char*)sb->last, str, len);
	sb->last += len - 1;
	((char*)sb->last)[1] = '\0';
}
/* Appends a formatted string using printf-style specifiers, resizing if necessary */
static void appendf(string_builder sb, string format, ...) {
	if (!sb || !format) return;
	va_list args;
	va_start(args, format);
	
	va_list args_copy;
	va_copy(args_copy, args);
	int len = vsnprintf(NULL, 0, format, args_copy);
	//printf("\nappendf: expected length=%d\n", len);
	
	va_end(args_copy);
	if (len < 0) {
		va_end(args);
		return;
	}
	size_t required_len = (size_t)len;
	size_t current_len = sb->last + 1 - (addr)sb->buffer; /* Fix offset */
	//printf("appendf: current length=%ld\n", current_len);
	
	if (sb->last + required_len + 1 >= sb->end) {
		//printf("appendf: resizing ");
		size_t old_capacity = sb->end - (addr)sb->buffer;
		size_t new_capacity = old_capacity ? old_capacity * 2 : 16;
		//printf("capacity=(%ld) -> (%ld) (curr_len + req_len=%ld)\n", old_capacity, new_capacity, current_len + required_len);
		
		while (new_capacity < current_len + required_len + 1) new_capacity *= 2;
		//printf("appendf: adj new_cap=%ld\n", new_capacity);
		
		char* new_buffer = Mem.alloc(new_capacity + 1);
		if (!new_buffer) {
			va_end(args);
			return;
		}
		//printf("appendf: allocated new buffer [%s]\n", new_buffer ? "TRUE" : "FALSE");
		
		ByteArray.copyTo(sb->buffer, new_buffer, current_len); /* Byte-based */
		ByteArray.clear(new_buffer + current_len, new_capacity + 1 - current_len); /* Byte-based */
		
		Mem.free(sb->buffer);
		sb->last = (addr)new_buffer + current_len - 1;
		sb->buffer = new_buffer;
		sb->end = (addr)new_buffer + new_capacity;
	}
	
	char* write_pos = (char*)sb->last + 1; /* Start after last char */
	//size_t pos = write_pos - sb->buffer;
	//printf("appendf: write_pos=%lu\n", pos);
	
	len = vsnprintf(write_pos, required_len + 1, format, args);
	sb->last = (addr)(write_pos + len - 1); /* Point to last char, not \0 */
	
	va_end(args);
}
/* Appends a string followed by a newline, or just a newline if str is NULL, resizing if necessary */
static void appendLine(string_builder sb, string str) {
	if (!sb) return;
	size_t len = str ? strlen(str) + 1 : 1; /* +1 for \n */
	size_t current_len = sb->last - (addr)sb->buffer + 1;
	if (sb->last + len + 1 >= sb->end) {
		size_t old_capacity = sb->end - (addr)sb->buffer;
		size_t new_capacity = old_capacity ? old_capacity * 2 : 16;
		while (new_capacity < current_len + len) new_capacity *= 2;
		char* new_buffer = Mem.alloc(new_capacity + 1);
		if (!new_buffer) return;
		Collections.copyTo((addr*)sb->buffer, (addr*)new_buffer, sb->last + 1);
		Collections.clear((addr*)(new_buffer + current_len), (addr)(new_buffer + new_capacity + 1));
		Mem.free(sb->buffer);
		sb->last = (addr)new_buffer + current_len - 1;
		sb->buffer = new_buffer;
		sb->end = (addr)new_buffer + new_capacity;
	}
	sb->last++;
	if (str) {
		memcpy((char*)sb->last, str, len - 1);
		sb->last += len - 2; /* Point to last char of str */
	}
	((char*)sb->last)[1] = '\n';
	((char*)sb->last)[2] = '\0';
	sb->last++; /* Now points to \n */
}
/* Resets the buffer to empty, clearing content and resetting last */
static void clear(string_builder sb) {
	if (!sb) return;
	Collections.clear((addr*)sb->buffer, sb->last + 1);
	sb->last = (addr)sb->buffer - 1;
	sb->buffer[0] = '\0';
}
/* Returns a new string with the current content, caller must free */
static string toString(string_builder sb) {
	if (!sb || !sb->buffer) return NULL;
	size_t len = getLength(sb);
	
	string result = Mem.alloc(len + 1);
	if (!result) return NULL;
	
	//printf("\n");
	//printf("source:   buffer=%lu last=%lu count=%ld\n", (addr)sb->buffer, sb->last, len);
	//printf("				 %p	   %p		%ld\n", sb->buffer, (char*)sb->last, (char*)sb->last - sb->buffer);
	//printf("dest:	 buffer=%lu last=%lu count=%ld\n", (addr)result, (addr)result + len, ((addr)result + len) - (addr)result);
	//printf("				 %p	   %p		%ld\n", (addr*)sb->buffer, (addr*)result, (addr)(result + len) - (addr)result);
	
	ByteArray.copyTo(sb->buffer, result, len + 1);
	//printf("result:   length=%ld\n", strlen(result));
	return result;
}
/* Writes the buffer contents to the given stream */
static void writeToStream(string_builder sb, FILE* stream) {
 if (!sb || !sb->buffer || !stream) return;
 size_t len = getLength(sb);
 
 if (len > 0) {
 	fwrite(sb->buffer, 1, len, stream);
 }
}
/* Returns the current number of characters in the buffer */
static size_t getLength(string_builder sb) {
	if (!sb) return 0;
	return sb->last - (addr)sb->buffer + 1;
}
/* Returns the total number of usable characters in the buffer */
static size_t getCapacity(string_builder sb) {
	if (!sb) return 0;
	return sb->end - (addr)sb->buffer;
}/* Returns an iterator for traversing the string builder’s buffer */
static iterator getRangeIterator(string_builder sb, int start, int count) {
	if (!sb || !sb->buffer || start < 0 || count < 0) return NULL;
	size_t total = getLength(sb);
	if (start >= total) return NULL;	// bounds check
	//	adjust count to actual range
	count = (count > (total - start)) ? (total - start) : count;
	
	object range_start = (char*)sb->buffer + start;
	object range_end = (char*)sb->buffer + start + count;
	return create_iterator(range_start, range_end, 1);
}
/* Adjusts the buffer capacity, preserving current content */
static void setCapacity(string_builder sb, size_t new_capacity) {
	if (!sb) return;
	size_t current_len = sb->last - (addr)sb->buffer + 1;
	if (new_capacity < current_len) return;
	char* new_buffer = Mem.alloc(new_capacity + 1);
	if (!new_buffer) return;
	Collections.copyTo((addr*)sb->buffer, (addr*)new_buffer, sb->last + 1);
	Collections.clear((addr*)(new_buffer + current_len), (addr)(new_buffer + new_capacity + 1));
	Mem.free(sb->buffer);
	sb->last = (addr)new_buffer + current_len - 1;
	sb->buffer = new_buffer;
	sb->end = (addr)new_buffer + new_capacity;
}
/* Frees the string builder and its buffer */
static void free_string_builder(string_builder sb) {
	if (!sb) return;
	Mem.free(sb->buffer);
	Mem.free(sb);
}

const IStringBuilder StringBuilder = {
	.new = newStringBuilder,
	.append = append,
	.appendf = appendf,
	.appendLine = appendLine,
	.clear = clear,
	.toString = toString,
	.toStream = writeToStream,
	.length = getLength,
	.capacity = getCapacity,
	.setCapacity = setCapacity,
	.free = free_string_builder,
	.iterateRange = getRangeIterator
};

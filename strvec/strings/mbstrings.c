#include "./mbstrings.h"

/* mbslen - multi-byte string length
 * - Description: returns the number of UTF-8 code points ("characters")
 * in a multibyte string. If the argument is NULL or an invalid UTF-8
 * string is passed, returns -1.
 *
 * - Arguments: A pointer to a character array (`src`), consisting of UTF-8
 * variable-length encoded multibyte code points.
 *
 * - Return: returns the actual number of UTF-8 code points in `src`. If an
 * invalid sequence of bytes is encountered, return -1.
 *
 * - Hints:
 * UTF-8 characters are encoded in 1 to 4 bytes. The number of leading 1s in the
 * highest order byte indicates the length (in bytes) of the character. For
 * example, a character with the encoding 1111.... is 4 bytes long, a character
 * with the encoding 1110.... is 3 bytes long, and a character with the encoding
 * 1100.... is 2 bytes long. Single-byte UTF-8 characters were designed to be
 * compatible with ASCII. As such, the first bit of a 1-byte UTF-8 character is
 * 0.......
 *
 * You will need bitwise operations for this part of the assignment!
 */
size_t mbslen(const char* bytes) {
  // # of code points in the UTF-8 string
  size_t count = 0;
  const char* ptr = bytes;
  if (ptr == NULL) {
    return -1;
  }
  while (*ptr != 0) {
	char c_128 = 128;
	char c_224 = 224;
	char c_192 = 192;
	char c_240 = 240;
	char c_248 = 248;
	char c_0 = 0;
    if ((*ptr & c_128) == c_0) {
	  count++;
	  ptr++;
	  continue;
	}
	const char* ptr_b = ptr + 1;
    if ((*ptr & c_224) == c_192) {
	  if ((*ptr_b & c_192) == c_128) {
	    count++;
		ptr = ptr + 2;
		continue;
	  } else {
	    return -1;
	  }
	}
	const char* ptr_c = ptr + 2;
	if ((*ptr & c_240) == c_224) {
	  if (((*ptr_b & c_192) == c_128) && ((*ptr_c & c_192) == c_128)) {
	    count++;
		ptr = ptr + 3;
		continue;
	  } else {
	    return -1;
	  }
	}
	const char* ptr_d = ptr + 3;
	if ((*ptr & c_248) == c_240) {
	  if (((*ptr_b & c_192) == c_128) && ((*ptr_c & c_192) == c_128) && ((*ptr_d & c_192) == c_128)) {
	    count++;
		ptr = ptr + 4;
		continue;
	  } else {
	    return -1;
	  }
	}
	// first byte is not a valid pattern.
	return -1;
  }
  return count;
}

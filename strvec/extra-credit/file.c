#include "file.h"
#include <stdlib.h>
#include <stdbool.h>

/* detect_encoding() - reads through the file indicated by fd
 * to determine its encoding
 *
 * Arguments - fd, the file descriptor for the desired file
 *
 * Return Value - returns the encoding of the file
 */
enum encoding_type detect_encoding(int fd) {
  // TODO: your implementation here!
  char buf[5];
  size_t num = 0;
  bool not_ascii = false;
  bool is_empty = true;
  while ((num = read(fd, buf, 1)) > 0) {
    is_empty = false;
    char a = 128;
    if ((buf[0] & a) == 0) {
      continue;
    }
    not_ascii = true;
    char b = 192;
    char c = 224;
    char d = 240;
    char e = 248;
    if ((buf[0] & c) == b) {
      num = read(fd, buf, 1);
      if (num < 1) {
        goto out;
      }
      if ((buf[0] & b) == a) {
        continue;
      } else {
        goto out;
      }
    }
    if ((buf[0] & d) == c) {
      num = read(fd, buf, 2);
      if (num < 2) {
        goto out;
      }
      if (((buf[0] & b) == a) && ((buf[1] & b) == a)) {
        continue;
      } else {
        goto out;
      }
    }
    if ((buf[0] & e == d)) {
      num = read(fd, buf, 3);
      if (num < 3) {
        goto out;
      }
      if (((buf[0] & b == a)) && ((buf[1] & b == a)) && ((buf[2] & b) == a)) {
        continue;
      } else {
        goto out;
      }
    }
  }
  if (is_empty) {
    goto out;
  }
  if (num < 0) {
    goto out;
  }
  if (not_ascii) {
    return UTF_8;
  } else {
    return ASCII;
  }
 out:
  return UNDEFINED;
  
}

/* The current implementation of main() opens the file and calls
 * `detect_encoding()`, then prints the result.
 *
 * We only ask you to make `detect_encoding()` work correctly,
 * but if you feel motivated for a stretch goal, can you make
 * this implementation match the output of the standard "file"
 * utility in Linux? Here's how file works:
 *
 * $ file strings.c
 * strings.c: C source, ASCII text
 * $ file emojis.txt
 * emojis.txt: UTF-8 Unicode text
 *
 * We provide some challenging examples. Run `make
 * check-stretch` to see if your implementation meets the
 * challenges!
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: file <PATH>\n");
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("open failed");
    return 1;
  }

  switch (detect_encoding(fd)) {
    case UNDEFINED:
      printf("Undefined\n");
      break;
    case ASCII:
      printf("ASCII\n");
      break;
    case UTF_8:
      printf("UTF-8\n");
      break;
  }

  if (close(fd) == -1) {
    perror("close");
    return 1;
  }

  return 0;
}

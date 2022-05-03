#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "../io300.h"



/*
    student.c

    Fill in the following stencils

*/

/*
    When starting, you might want to change this for testing on small files.
*/
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 20
#endif

#if(BUFFER_SIZE < 4)
#error "internal buffer size should not be below 4."
#error "if you changed this during testing, that is fine."
#error "when handing in, make sure it is reset to the provided value"
#error "if this is not done, the autograder will not run"
#endif

/*
   This macro enables/disables the dbg() function. Use it to silence your
   debugging info.

   Use the dbg() function instead of printf debugging if you don't want to
   hunt down 30 printfs when you want to hand in
*/
#define DEBUG_PRINT 0
#define DEBUG_STATISTICS 0

enum RW_FLAG {OPEN, READ, WRITE, SEEK};

struct io300_file {
    /* read,write,seek all take a file descriptor as a parameter */
    int fd;
    /* this will serve as our cache */
    char *buffer;


    // TODO: Your properties go here
    enum RW_FLAG rw_flag;
    off_t cur_off;
    int read_index;
    int read_buf_end;
    int write_index;
    int write_buf_end;

    /* Used for debugging, keep track of which io300_file is which */
    char *description;
    /* To tell if we are getting the performance we are expecting */
    struct io300_statistics {
        int read_calls;
        int write_calls;
        int seeks;
    } stats;
};

/*
    Assert the properties that you would like your file to have at all times.
    Call this function frequently (like at the beginning of each function) to
    catch logical errors early on in development.
*/
static void check_invariants(struct io300_file *f) {
    assert(f != NULL);
    assert(f->buffer != NULL);
    assert(f->fd >= 0);

    // TODO: Add more invariants
}

/*
    Wrapper around printf that provides information about the
    given file. You can silence this function with the DEBUG_PRINT macro.
*/
static void dbg(struct io300_file *f, char *fmt, ...) {
    (void)f; (void)fmt;
#if(DEBUG_PRINT == 1)
    static char buff[300];
    size_t const size = sizeof(buff);
    int n = snprintf(
        buff,
        size,
        // TODO: Add the fields you want to print when debugging
        "{desc:%s, } -- ",
        f->description
    );
    int const bytes_left = size - n;
    va_list args;
    va_start(args, fmt);
    vsnprintf(&buff[n], bytes_left, fmt, args);
    va_end(args);
    printf("%s", buff);
#endif
}



struct io300_file *io300_open(const char *const path, char *description) {
    if (path == NULL) {
        fprintf(stderr, "error: null file path\n");
        return NULL;
    }

    int const fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: `%s`: %s\n", path, strerror(errno));
        return NULL;
    }

    struct io300_file *const ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        fprintf(stderr, "error: could not allocate io300_file\n");
        return NULL;
    }

    ret->fd = fd;
    ret->buffer = malloc(BUFFER_SIZE);
    if (ret->buffer == NULL) {
        fprintf(stderr, "error: could not allocate file cache\n");
        close(ret->fd);
        free(ret);
        return NULL;
    }
    ret->description = description;
    // TODO: Initialize your file
    ret->rw_flag = OPEN;
    ret->read_index = -1;
    ret->read_buf_end = -1;
    ret->write_index = -1;
    ret->cur_off = 0;
    check_invariants(ret);
    dbg(ret, "Just finished initializing file from path: %s\n", path);
    return ret;
}

int io300_seek(struct io300_file *const f, off_t const pos) {
    check_invariants(f);
    f->stats.seeks++;

    // TODO: Implement this
    if (f->rw_flag == WRITE) {
      int flush_flag = io300_flush(f);
      if (flush_flag < 0) {
        return flush_flag;
      }
    }
    f->cur_off =  lseek(f->fd, pos, SEEK_SET);
    f->rw_flag = SEEK;
    return f->cur_off;
}

int io300_close(struct io300_file *const f) {
    check_invariants(f);

#if(DEBUG_STATISTICS == 1)
    printf("stats: {desc: %s, read_calls: %d, write_calls: %d, seeks: %d}\n",
            f->description, f->stats.read_calls, f->stats.write_calls, f->stats.seeks);
#endif
    // TODO: Implement this
    int flush_flag = io300_flush(f);
    close(f->fd);
    free(f->buffer);
    free(f);
    return flush_flag;
}

off_t io300_filesize(struct io300_file *const f) {
    check_invariants(f);
    struct stat s;
    int const r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}

int io300_readc(struct io300_file *const f) {
    check_invariants(f);
    // TODO: Implement this
    unsigned char c;
    if (io300_read(f, (char*)&c, 1) == 1) {
        return (int)c;
    } else {
        return -1;
    }
}

int io300_writec(struct io300_file *f, int ch) {
    check_invariants(f);
    // TODO: Implement this
    char const c = (char)ch;
    return io300_write(f, &c, 1) == 1 ? 1 : -1;
}

ssize_t io300_read(struct io300_file *const f, char *const buff, size_t const sz) {
    check_invariants(f);
    // TODO: Implement this
    if (f->rw_flag == WRITE) {
      int flush_flag = io300_flush(f);
      if (flush_flag < 0) {
        return -1;
      }
      f->read_buf_end = -1;
      f->read_index = -1;
      f->cur_off = lseek(f->fd, 0, SEEK_CUR);
    }
    if (f->rw_flag == SEEK) {
      ssize_t read_sz = read(f->fd, f->buffer, BUFFER_SIZE);
      if (read_sz < 0) {
        return -1;
      }
      f->read_index = -1;
      f->read_buf_end = read_sz - 1;
    }
    f->rw_flag = READ;
    size_t remain_sz = f->read_buf_end - f->read_index;
    if (sz > BUFFER_SIZE) {
      memcpy(buff, f->buffer + f->read_index + 1, remain_sz);
      f->read_index = -1;
      f->read_buf_end = -1;
      f->cur_off += remain_sz;
      lseek(f->fd, f->cur_off, SEEK_SET);
      size_t next_sz = sz - remain_sz;
      ssize_t read_sz = read(f->fd, buff + remain_sz, next_sz);
      if (read_sz < 0) {
        return remain_sz;
      } else {
	f->cur_off += read_sz;
        return remain_sz + read_sz;
      }
    }
    if (sz > remain_sz) {
      memcpy(buff, f->buffer + f->read_index + 1, remain_sz);
      size_t next_sz = sz - remain_sz;
      f->cur_off += remain_sz;
      lseek(f->fd, f->cur_off, SEEK_SET);
      ssize_t read_sz = read(f->fd, f->buffer, BUFFER_SIZE);
      if (read_sz < 0) {
	f->read_index = -1;
	f->read_buf_end = -1;
        return remain_sz;
      }
      f->read_index = -1;
      f->read_buf_end = read_sz - 1;
      char *const buffer_ptr = buff + remain_sz;
      if (next_sz < (size_t)(f->read_buf_end - f->read_index)) {
        memcpy(buffer_ptr, f->buffer, sz);
	f->read_index += next_sz;
	f->cur_off += next_sz;
	return next_sz + remain_sz;
      } else {
	int copy_sz = f->read_buf_end - f->read_index;
        memcpy(buffer_ptr, f->buffer, copy_sz);
	f->read_index = -1;
	f->read_buf_end = -1;
	f->cur_off += copy_sz;
	return copy_sz + remain_sz;
      }
    }
    memcpy(buff, f->buffer + f->read_index + 1, sz);
    f->read_index += sz;
    f->cur_off += sz;
    return sz;
}

ssize_t io300_write(struct io300_file *const f, const char *buff, size_t const sz) {
    check_invariants(f);
    // TODO: Implement this
    if (f->rw_flag == READ) {
      lseek(f->fd, f->cur_off, SEEK_SET);
    }
    f->rw_flag = WRITE;
    if (f->write_index + sz < BUFFER_SIZE) {
      memcpy(f->buffer + f->write_index + 1, buff, sz);
      f->write_index = f->write_index + sz;
      f->cur_off += sz;
      return sz;
    }
    ssize_t write_sz = write(f->fd, f->buffer, f->write_index + 1);
    if (write_sz < f->write_index + 1) {
      if (write_sz < 0) {
        return -1;
      }
      memcpy(f->buffer, f->buffer + write_sz, f->write_index + 1 - write_sz);
      f->write_index = f->write_index - write_sz;
      return -1;
    }
    f->write_index = -1;
    write_sz = write(f->fd, buff, sz);
    if (write_sz >= 0) {
      f->cur_off += write_sz;
    }
    return write_sz;
}

int io300_flush(struct io300_file *const f) {
    check_invariants(f);
    // TODO: Implement this
    if (f->rw_flag == WRITE) {
      ssize_t write_sz = write(f->fd, f->buffer, f->write_index + 1);
      if (write_sz > 0) {
        f->write_index = f->write_index - write_sz;
      }
      if (f->write_index == -1) { //indicate flush all date to disk.
        return 0;
      } else {
        return -1;
      }
    }
    return 0;
}

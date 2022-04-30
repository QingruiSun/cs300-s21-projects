#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <cassert>
#include <cstring>
#include <unordered_map>

#define ALIGNMENT(sz) ((sz + 7) & (~7))
#define GET_STATE_PTR(ptr) (void*)((uintptr_t)ptr - 8)
#define GET_SIZE_PTR(ptr) (void*)((uintptr_t)ptr - 16)
#define ADVANCE_PTR(ptr) (void*)((uintptr_t)ptr + 16)
#define BACK_PTR(ptr) (void*)((uintptr_t)ptr - 16)

struct dmalloc_stats global_stats = {0, 0, 0, 0, 0, 0, 0, 0};
// magic number indicates the memory block state.
const int MALLOC = 4354534;
const int FREED = 88355;
const char end_guard = 157;

std::unordered_map<uintptr_t, int> states_map;
std::unordered_map<uintptr_t, size_t> size_map;
std::unordered_map<uintptr_t, position> position_map;

/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and 
 *      return a pointer to it 
 * 
 * @arg size_t sz : the amount of memory requested 
 * @arg const char *file : a string containing the filename from which dmalloc was called 
 * @arg long line : the line number from which dmalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    void* ptr;
    void* end_guard_ptr;
    position pos;
    size_t alloc_size = sz + 17;
    if (alloc_size < sz) {
      goto fail;
    }
    ptr =  base_malloc(alloc_size);
    if (!ptr) {
      goto fail;
    }
    global_stats.ntotal += 1;
    global_stats.total_size += (unsigned long long)sz;
    global_stats.nactive += 1;
    global_stats.active_size += (unsigned long long)sz;
    ptr = ADVANCE_PTR(ptr);
    states_map[(uintptr_t)ptr] = MALLOC;
    size_map[(uintptr_t)ptr] = sz;
    pos.file = (char *)file;
    pos.line = line;
    position_map[(uintptr_t)ptr] = pos;
    memcpy(GET_SIZE_PTR(ptr), &sz, sizeof(size_t));
    memcpy(GET_STATE_PTR(ptr), &MALLOC, sizeof(int));
    end_guard_ptr = (void*)((uintptr_t)ptr + sz);
    memcpy(end_guard_ptr, &end_guard, 1);
    if (!global_stats.heap_min) {
      global_stats.heap_min = (uintptr_t)ptr;
    } else {
      global_stats.heap_min = global_stats.heap_min < (uintptr_t)ptr ? global_stats.heap_min : (uintptr_t)ptr;
    }
    if (!global_stats.heap_max) {
      global_stats.heap_max = (uintptr_t)ptr + sz;
    } else {
      global_stats.heap_max = global_stats.heap_max > (uintptr_t)ptr + sz ? global_stats.heap_max : (uintptr_t)ptr + sz;
    }
    return ptr;
fail:
    global_stats.nfail += 1;
    global_stats.fail_size += (unsigned long long)sz;
    return NULL;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    if (!ptr) {
      return;
    }
    (void) file, (void) line;
    // Your code here.
    void* size_ptr;
    size_t size;
    void* state_ptr;
    void* end_guard_ptr;
    position pos;
    size_t original_size;
    long distance;
    if (states_map.count((uintptr_t)ptr) > 0) {
      if (states_map[(uintptr_t)ptr] == FREED) {
        goto double_free;
      }
    } else {
      for (const auto& [key, value] : states_map) {
        if (((uintptr_t)ptr >= key) && (value == MALLOC) && ((uintptr_t)ptr < key + size_map[key])) {
          pos = position_map[key];
	  original_size = size_map[key];
	  distance = (long)((uintptr_t)ptr - key);
	  goto not_allocated;
	}
      }
      goto not_in_heap;
    }
    size_ptr = GET_SIZE_PTR(ptr);
    size = *((size_t*)(size_ptr));
    global_stats.nactive -= 1;
    global_stats.active_size -= size;
    state_ptr = GET_STATE_PTR(ptr);
    memcpy(state_ptr, &FREED, sizeof(int));
    states_map[(uintptr_t)ptr] = FREED;
    end_guard_ptr = (void*)((uintptr_t)ptr + size);
    if (memcmp(end_guard_ptr, &end_guard, 1) != 0) {
      goto write_error;
    }
    ptr = BACK_PTR(ptr);
    base_free(ptr);
    return;
write_error:
    fprintf(stderr, "MEMORY BUG: %s:%ld: detected wild write during free of pointer %p", file, line, ptr);
    abort();
    return;
not_in_heap:
    fprintf(stderr, "MEMORY BUG: %s:%ld: invalid free of pointer %p, not in heap\n", file, line, ptr);
    abort();
    return;
not_allocated:
    fprintf(stderr, "MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated\n", file, line, ptr);
    fprintf(stderr, "%s:%ld: %p is %ld bytes inside a %ld byte region allocated here\n", pos.file, pos.line, ptr, distance, original_size);
    abort();
    return;
double_free:
    fprintf(stderr, "MEMORY BUG: %s:%ld: invalid free of pointer %p, double free\n", file, line, ptr);
    abort();
    return;
}

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb` 
 *      number of elements with wach element being `sz` bytes. The memory should be initialized 
 *      to zero  
 * 
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called 
 * @arg long line : the line number from which dcalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    size_t alloc_size = nmemb * sz;
    //judge integer iverflow.
    if (alloc_size / sz != nmemb) {
      global_stats.nfail += 1;
      global_stats.fail_size += (unsigned long long)nmemb * (unsigned long long)sz;
      return NULL;
    }
    void* ptr = dmalloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_stats));
    // Your code here.
    memcpy(stats, &global_stats, sizeof(dmalloc_stats));
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
    for (const auto& [key, value] : states_map) {
      if (value == MALLOC) {
	position pos = position_map[key];
        printf("LEAK CHECK: %s:%ld: allocated object %p with size %ld\n", pos.file, pos.line, (void*)key, size_map[key]);
      }
    }
}

#include "vector300.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Initializes all of the fields of a vector struct
 *Arguments:
 * - v: a pointer to a vector_t
 * - type: a size_t indicating the size of the type of this vector's elements
 *Return value: none
 */
void initialize_vector(vector_t* v, size_t type) {
  // TODO: implement
  v->array = malloc(2 * type);  // you'll need to change this
  v->capacity = 2;
  v->ele_size = type;
  v->length = 0;
}

/*Frees this vector
 *Arguments:
 * - v: a pointer to the vector which needs to be freed
 *Return value: none
 */
void destroy_vector(vector_t* v) {
  // TODO: implement
  free(v->array);
  v->array = NULL;
  v->capacity = 0;
  v->length = 0;
}

/*Gets the size of a given vector
 *Arguments:
 * - v: the vector whose size is desired
 *Return value: an integer containing the size of the vector
 */
size_t vector_size(vector_t* v) {
  // TODO: implement
  return v->length;
}

/*Gets the element at a desired position within a vector
 *Arguments:
 * - v: a pointer to a vector_t
 * - index: the index of the desired element in v (with 0 indexing)
 *Return value: a void pointer to the element at index (to be casted
 *appropriately by the caller)
 */
void* vector_get(vector_t* v, int index) {
  // TODO: implement
  if ((size_t)index > v->length) {
    return NULL;
  }
  return v->array + ((size_t)index * v->ele_size);
}

/*Adds an element to the back of a vector, doubling the capacity of the vector
 *if needed Arguments:
 * - v: a pointer to a vector_t
 * - ele: a pointer to the element to be copied into v
 * Return value: none
 */
void vector_add_back(vector_t* v, void* ele) {
  // TODO: implement
  if (v->length == v->capacity) {
    v->array = realloc(v->array, v->capacity * 2 * v->ele_size);
    v->capacity *= 2;
  }
  memcpy(v->array + v->ele_size * v->length, ele, v->ele_size);
  v->length += 1;
}

/*Removes the last element in a vector
 *Arguments:
 * - v: a pointer to a vector_t
 *Return value: none
 */
void vector_delete_back(vector_t* v) {
  // TODO: implement
  v->length -= 1;
}

/*Adds an element to a specified index in a vector, double its capacity if
 *needed Arguments:
 * - v: a pointer to a vector_t
 * - ele: a pointer to the element to be copied into v
 * - index: the desired index for ele in the vector, v (using 0 indexing)
 *Return value: none
 */
void vector_add(vector_t* v, void* ele, int index) {
  // TODO: implement
  if ((size_t)index > v->length) {
    return;
  }
  if (v->length == v->capacity) {
    v->array = realloc(v->array, v->capacity * 2 * v->ele_size);
    v->capacity *= 2;
  }
  for (size_t i = v->length; i > (size_t)index; --i) {
    memcpy(v->array + i * v->ele_size, v->array + (i - 1) * v->ele_size, v->ele_size);
  }
  memcpy(v->array + (size_t)index * v->ele_size, ele, v->ele_size);
  v->length += 1;
}

/*Deletes an element from the specified position in a vector
 *Arguments:
 * - v: a pointer to a vector_t
 * - index: the index of the element to be deleted from v (using 0 indexing)
 *Return value: none
 */
void vector_delete(vector_t* v, int index) {
  // TODO: implement
  if ((size_t)index >= v->length) {
    return;
  }
  for (size_t i = (size_t)index + 1; i < v->length; ++i) {
    memcpy(v->array + (i - 1) * v->ele_size, v->array + i * v->ele_size, v->ele_size);
  }
  v->length -= 1;
}

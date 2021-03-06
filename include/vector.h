#ifndef __VECTOR_H__
#define __VECTOR_H__
/**@file vector.h 
 * @brief A simple vector, which can dynamic resize at run time */
#include <stdint.h>
#include <stdlib.h>
#include <constants.h>
/** @brief data struct for vector */
typedef struct {
	size_t  capacity;       /*!<Current Capacity of the vector, used for resize */
	size_t  size;           /*!<Current used size of vector */
	size_t  elem_size;      /*!<Indicates the size of element */
	char*   data;           /*!<The data section */
} vector_t;
/** @brief create a new vector 
 *  @param elem_size the size of each element
 *	@return the vector created, NULL means error 
 */
vector_t* vector_new(size_t elem_size);  /* Create a new vector */
/** @brief free a vector 
 *  @param vec vector that to be freed
 *  @return nothing */
void      vector_free(vector_t* vec);      /* free the vector after use */
/** @brief push an element at the end of the vector, if the vector is full, resize first
 *  @param vec vector 
 *  @param data data to be stored
 *  @return result of operation >=0 means success
 */
int       vector_pushback(vector_t* vec, void* data);  /* push an element at the end of the vector */

/** @brief Get the element idx in the vector
 *  @param vec vector
 *  @param idx index 
 *  @return a pointer to the element, NULL means error
 */
static inline void* vector_get(const vector_t* vec, int idx) 
{
#ifdef CHECK_EVERYTHING   /* For performance reason, we DO NOT check the validity of vector. */
	if(NULL == vec) return NULL; 
	if(idx >= vec->size) return NULL;
#endif
	return (void*)(vec->data + vec->elem_size * idx);
}
/** @brief return the size(number of elements) of the vector
 *  @param vec vector
 *  @return size of vector
 */
static inline size_t vector_size(const vector_t* vec)
{
#ifdef CHECK_EVERYTHING
	if(NULL == vec) return 0;
#endif
	return vec->size;
}
/**
 * @brief init the next empty cell, but do not perform assignment
 * @param vector the vector
 * @return < 0 for error
 **/
int vector_dry_pushback(vector_t* vector);
#endif /* __VECTOR_H__*/

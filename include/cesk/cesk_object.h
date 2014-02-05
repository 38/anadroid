#ifndef __CESK_OBJECT_H__
#define __CESK_OBJECT_H__
#include <constants.h>

#include <cesk/cesk_value.h>
/* a abstract object value */
typedef struct {
    const char*              classpath;   /* the class path of this object */
    size_t                   num_members; /* the number of members */
    cesk_value_set_t*        valuelist[0];  /* the value of the member */
} cesk_object_struct_t;  

typedef struct {
    uint16_t         depth;      /* the depth in inherence tree */
    cesk_object_struct_t  members[0]; /* the length of the tree */
} cesk_object_t;


/* Create a new instance object of class in <classpath> */
cesk_object_t*   cesk_object_new(const char* classpath);
/* Get address of a dynamic field in an object, 
 * this function is to return a reference to the cell contains the value*/
cesk_value_set_t** cesk_object_get(cesk_object_t* object, const char* classpath, const char* field);

/* get the hash code of a object */
hashval_t cesk_object_hashcode(cesk_object_t* object); 
#endif
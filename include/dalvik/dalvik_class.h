#ifndef __DALVIK_CLASS_H__
#define __DALVIK_CLASS_H__
#include <sexp.h>
#include <dalvik/dalvik_attrs.h>


typedef struct{
    const char* path;
    const char* super;              /* The class path of super class */
    const char* implements[128];         /* The interface path that the class implements */
    int         attrs;
    int         is_interface;
    const char* members[0];    /* We use variant length structure to represents a class */
} dalvik_class_t;

/* 
 * Build a new class from a S-Expression
 * The reason why we don't need a destructor is
 * all class is managed by memberdict automaticly.
 * So the member take by all class definition and
 * all method and field will be desopsed when the 
 * memberdict finalization function is called 
 */
dalvik_class_t* dalvik_class_from_sexp(sexpression_t* sexp);

#endif

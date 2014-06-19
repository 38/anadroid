/**
 * @brief this is the interface provided by BCI subsystem to 
 *        built-in classes.
 **/
#ifndef __BCI_INTERFACE_H__
#define __BCI_INTERFACE_H__
#include <bci/bci.h>
#include <stringpool.h>
#define PackageInit_Begin \
int buildin_library_init()\
{

#define PackageInit_End \
	return 0;\
}\
void builtin_library_finalize()\
{\
}

#define Provides_Class(module_name) do{\
	extern bci_class_t module_name##_metadata;\
	uint32_t i;\
	for(i = 0; NULL != module_name##_metadata.provides[i]; i ++) {\
		module_name##_metadata.provides[i] = stringpool_query(module_name##_metadata.provides[i]);\
		if(bci_nametab_register_class(module_name##_metadata.provides[i], &module_name##_metadata) < 0)\
		{\
			LOG_ERROR("Can not register built-in class %s", module_name##_metadata.provides[i]);\
			return -1;\
		}\
		else\
		{\
			LOG_DEBUG("Registered built-in class %s", module_name##_metadata.provides[i]);\
			if(NULL != module_name##_metadata.onload)\
				module_name##_metadata.onload(module_name##_metadata.provides[i]);\
		}\
	}\
}while(0)
#endif
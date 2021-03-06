#include <dalvik/dalvik_memberdict.h>
#include <log.h>
#include <stringpool.h>
#include <string.h>
#include <dalvik/dalvik_class.h>
#include <dalvik/dalvik_method.h>
#include <dalvik/dalvik_field.h>
#include <debug.h>

#define _TYPE_METHOD 0
#define _TYPE_FIELD 1
#define _TYPE_CLASS 2
/**
 * @brief the node of hash table for member dictionary
 **/
typedef struct _dalvik_memberdict_node_t {
	const char* class_path;             /*!< the class path */
	const char* member_name;            /*!< the name of the member */
	const dalvik_type_t* const * args;  /*!< the type list (only valid for method, otherwise set to NULL */
	const dalvik_type_t* rtype;         /*!< the return value of this function, only valid for method, otherwise set to nULL */
	int         type;                   /*!< type of the object method, field or class */
	void*       object;                 /*!< the storage of the object */
	struct _dalvik_memberdict_node_t* next;  /*!< the next node */
} dalvik_memberdict_node_t;

dalvik_memberdict_node_t* _dalvik_memberdict_hash_table[DALVIK_MEMBERDICT_SIZE];

int dalvik_memberdict_init()
{
	memset(_dalvik_memberdict_hash_table, 0, sizeof(_dalvik_memberdict_hash_table));
	return 0;
}
void dalvik_memberdict_finalize()
{
	int i;
	for(i = 0; i < DALVIK_MEMBERDICT_SIZE; i ++)
	{
		dalvik_memberdict_node_t* ptr;
		for(ptr = _dalvik_memberdict_hash_table[i];
			NULL != ptr;)
		{
			dalvik_memberdict_node_t* old = ptr;
			ptr = ptr->next;
			switch(old->type)
			{
				case _TYPE_METHOD:
					dalvik_method_free((dalvik_method_t*)old->object);
					break;
				case _TYPE_FIELD:
					dalvik_field_free((dalvik_field_t*)old->object);
					break;
				case _TYPE_CLASS:
					/* class type is just a simple list */
					free(old->object); 
					break;
				default:
					LOG_WARNING("unknown node type in member dict, do not know how to free it, try to free it directly");
					free(old->object);
			}
			free(old);
		}
	}
}
/**
 * @brief check wether or not p is a prefix of s
 * @param p
 * @param s
 * @return 1 for yes, 0 for no
 **/
static inline int _dalvik_memberdict_check_prefix(const char* p, const char* s)
{
	int ret = 1;
	if(NULL == p) return NULL == s;
	for(;*p && s && *s && ret; ret = (*(p++) == *(s++)));
	return ret;
}
/**
 * @brief get the list of members
 * @param class_path_prefix the prefix of the class path
 * @param member_name_prefix the prefix of the member name
 * @param p_class_path the buffer to return the class path
 * @param p_member_name the buffer to return the member name
 * @param p_signature the buffer to return the signature
 * @param p_return_type the buffer to return the return type
 * @param bufsize the size of the buffer
 * @return the number of objects returned , < 0 indicates error
 **/
static inline int _dalvik_memberdict_member_match(
		const char* class_path_prefix, 
		const char* member_name_prefix,
		const char** p_class_path,
		const char** p_member_name,
		const dalvik_type_t * const ** p_signature,
		const dalvik_type_t ** p_return_type,
		size_t bufsize)
{
	int ret = 0;
	int i;
	for(i = 0; i < DALVIK_MEMBERDICT_SIZE && ret < bufsize; i ++)
	{
		dalvik_memberdict_node_t* ptr;
		for(ptr = _dalvik_memberdict_hash_table[i]; NULL != ptr; ptr = ptr->next)
		{
			if(_dalvik_memberdict_check_prefix(class_path_prefix, ptr->class_path) &&
			   _dalvik_memberdict_check_prefix(member_name_prefix, ptr->member_name))
			{
				if(NULL != p_class_path) p_class_path[ret] = ptr->class_path;
				if(NULL != p_member_name) p_member_name[ret] = ptr->member_name;
				if(NULL != p_signature) p_signature[ret] = ptr->args;
				if(NULL != p_return_type) p_return_type[ret] = ptr->rtype;
				ret ++;
			}
		}
	}
	return ret;
}

/**
 * @brief the hashcode for the members 
 * @param class_path the class path for this object
 * @param member_name the name of this member, if this object is a class defination this field is NULL
 * @param args the input argument list, only valid for method defs, otherwise keep NULL
 * @param rtype the return type of this function, only valid for method defs, otherwise keep NULL
 * @param type the type of this record
 * @return the hash value of this object
 **/
static inline hashval_t _dalvik_memberdict_hash(
		const char* class_path, 
		const char* member_name, 
		const dalvik_type_t* const * args,
		const dalvik_type_t* rtype,
		int type)
{
	/* it's fine when running on a 32 bit machine */
	static const uint32_t typeid[] = {0xe3deful, 0x12fcdeul, 0x2323feul};
	hashval_t a = ((uintptr_t)class_path) & 0xffffffff;
	hashval_t b = ((uintptr_t)member_name) & 0xffffffff;
	hashval_t c = dalvik_type_list_hashcode(args);
	hashval_t d = dalvik_type_hashcode(rtype);
	return (a * a * 100003 * MH_MULTIPLY + b * MH_MULTIPLY + c + d * d * d)^typeid[type];
}
/**
 * @brief register a new member dictionary object
 * @param class_path the class path
 * @param object_name the object name of this object
 * @param args the argument list
 * @param rtype the return type (only used by methods)
 * @param type indicates which type of object is this object (could be class/method/field)
 * @param obj  the actuall object address
 * @return < 0 indicates error
 **/
static inline int _dalvik_memberdict_register_object(
		const char* class_path, 
		const char* object_name, 
		const dalvik_type_t * const * args ,
		const dalvik_type_t* rtype,
		int type, void* obj)
{
	uint32_t idx = _dalvik_memberdict_hash(class_path, object_name, args, rtype, type)%DALVIK_MEMBERDICT_SIZE;
	dalvik_memberdict_node_t* ptr;
	/* try to find the object in the hash table, if the object is found, that means that
	 * the we can not distinguish two object by their name and type. 
	 * This must be an mistake */
	for(ptr = _dalvik_memberdict_hash_table[idx]; NULL != ptr; ptr = ptr->next)
	{
		if(ptr->class_path == class_path && 
		   ptr->member_name == object_name && 
		   dalvik_type_list_equal(args, ptr->args) &&
		   dalvik_type_equal(rtype, ptr->rtype))
		{
			LOG_ERROR("can not register object %s.%s twice", class_path, object_name);
			return -1;
		}
	}
	ptr = (dalvik_memberdict_node_t*)malloc(sizeof(dalvik_memberdict_node_t));
	ptr->class_path = class_path;
	ptr->member_name = object_name;
	ptr->args = args;
	ptr->rtype = rtype;
	ptr->object = obj;
	ptr->type = type;
	ptr->next = _dalvik_memberdict_hash_table[idx];
	_dalvik_memberdict_hash_table[idx] = ptr;
	switch(type)
	{
		case _TYPE_CLASS:
			LOG_DEBUG("class %s is registered", ptr->class_path);
			break;
		case _TYPE_FIELD:
			LOG_DEBUG("field %s.%s is registered", ptr->class_path, ptr->member_name);
			break;
		case _TYPE_METHOD:
			LOG_DEBUG("method %s %s.%s%s is registered", 
					   dalvik_type_to_string(ptr->rtype, NULL, 0),
					   ptr->class_path,
					   ptr->member_name,
					   dalvik_type_list_to_string(ptr->args, NULL, 0));
			break;
		default:
			LOG_WARNING("unknown object is registed");
	}
	return 0;
}

int dalvik_memberdict_register_method(const char* class_path, dalvik_method_t* method)
{
	if(NULL == method) return -1;
	return _dalvik_memberdict_register_object(class_path, method->name, method->args_type, method->return_type, _TYPE_METHOD, method);
}


int dalvik_memberdict_register_field(const char* class_path, dalvik_field_t* field)
{
	if(NULL == field) return -1;
	return _dalvik_memberdict_register_object(class_path, field->name, NULL, NULL, _TYPE_FIELD, field);
}

int dalvik_memberdict_register_class(const char* class_path, dalvik_class_t* class)
{
	if(NULL == class) return -1;
	return _dalvik_memberdict_register_object(class_path, NULL, NULL, NULL, _TYPE_CLASS, class);
}
/** 
 * @brief find an object from the member dict with key <classpath, name, typelist, return_type> 
 * @param path path of the class that we want to find
 * @param name the name of this member
 * @param args the argument type list for this method, if this object is not a method, just fill NULL here
 * @param rtype the return type for this method, if this object is not a method, just fill NULL instead
 * @param type the object type class/method/field
 * @return a pointer to this object
 **/
static inline const void* _dalvik_memberdict_find_object(
		const char* path, 
		const char* name, 
		const dalvik_type_t * const * args, 
		const dalvik_type_t * rtype,
		int type)
{
	uint32_t idx = _dalvik_memberdict_hash(path, name, args, rtype, type)%DALVIK_MEMBERDICT_SIZE;
	dalvik_memberdict_node_t* ptr;
	for(ptr = _dalvik_memberdict_hash_table[idx];
		NULL != ptr;
		ptr = ptr->next)
	{
		if(ptr->class_path == path && 
		   ptr->member_name == name && 
		   dalvik_type_list_equal(ptr->args, args) &&
		   dalvik_type_equal(ptr->rtype, rtype))  
		{
			return ptr->object;
		}
	}
	return NULL;
}


const dalvik_method_t* dalvik_memberdict_get_method(
		const char* class_path, 
		const char* name, 
		const dalvik_type_t * const * args,
		const dalvik_type_t * rtype)
{
	return (const dalvik_method_t*)_dalvik_memberdict_find_object(class_path, name, args, rtype,_TYPE_METHOD);
}

const dalvik_field_t* dalvik_memberdict_get_field(const char* class_path, const char* name)
{
	return (const dalvik_field_t*)_dalvik_memberdict_find_object(class_path, name, NULL, NULL, _TYPE_FIELD);
}

const dalvik_class_t* dalvik_memberdict_get_class(const char* class_path)
{
	return (const dalvik_class_t*)_dalvik_memberdict_find_object(class_path, NULL, NULL, NULL, _TYPE_CLASS);
}
int dalvik_memberdict_find_class_by_prefix(const char* prefix, const char** p_class_path, size_t bufsize)
{
	return _dalvik_memberdict_member_match(prefix, NULL, p_class_path, NULL, NULL, NULL, bufsize);
}
int dalvik_memberdict_find_member_by_prefix(
		const char* class_prefix,
		const char* method_prefix,
		const char** p_class_path,
		const char** p_method_name,
		const dalvik_type_t * const ** p_signature,
		const dalvik_type_t ** p_rettype,
		size_t bufsize)
{
	return _dalvik_memberdict_member_match(class_prefix, method_prefix, p_class_path, p_method_name, p_signature, p_rettype, bufsize);
}

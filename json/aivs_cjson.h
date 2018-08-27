/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
//#include "iperf_config.h"

#ifndef AIVS_cJSON__h
#define AIVS_cJSON__h

#define HAVE_STDINT_H
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* project version */
#define AIVS_CJSON_VERSION_MAJOR 1
#define AIVS_CJSON_VERSION_MINOR 5
#define AIVS_CJSON_VERSION_PATCH 2

#include <stddef.h>

/* cJSON Types: */
#define aivs_cJSON_Invalid (0)
#define aivs_cJSON_False  (1 << 0)
#define aivs_cJSON_True   (1 << 1)
#define aivs_cJSON_NULL   (1 << 2)
#define aivs_cJSON_Number (1 << 3)
#define aivs_cJSON_String (1 << 4)
#define aivs_cJSON_Array  (1 << 5)
#define aivs_cJSON_Object (1 << 6)
#define aivs_cJSON_Raw    (1 << 7) /* raw json */
#define aivs_cJSON_NumberLL (1 << 8)

#define aivs_cJSON_IsReference 256
#define aivs_cJSON_StringIsConst 512

/* The aivs_cJSON structure: */
typedef struct aivs_cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct aivs_cJSON *next;
    struct aivs_cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct aivs_cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==aivs_cJSON_String  and type == aivs_cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use aivs_cJSON_SetNumberValue instead */
    int64_t valueint;
    /* The item's number, if type==aivs_cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} aivs_cJSON;

typedef struct aivs_cJSON_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} aivs_cJSON_Hooks;

typedef int aivs_cJSON_bool;

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif
#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 2 define options:

AIVS_CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
AIVS_CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
AIVS_CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the AIVS_CJSON_API_VISIBILITY flag to "export" the same symbols the way AIVS_CJSON_EXPORT_SYMBOLS does

*/

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(AIVS_CJSON_HIDE_SYMBOLS) && !defined(AIVS_CJSON_IMPORT_SYMBOLS) && !defined(AIVS_CJSON_EXPORT_SYMBOLS)
#define AIVS_CJSON_EXPORT_SYMBOLS
#endif

#if defined(AIVS_CJSON_HIDE_SYMBOLS)
#define AIVS_CJSON_PUBLIC(type)   type __stdcall
#elif defined(AIVS_CJSON_EXPORT_SYMBOLS)
#define AIVS_CJSON_PUBLIC(type)   __declspec(dllexport) type __stdcall
#elif defined(AIVS_CJSON_IMPORT_SYMBOLS)
#define AIVS_CJSON_PUBLIC(type)   __declspec(dllimport) type __stdcall
#endif
#else /* !WIN32 */
#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(AIVS_CJSON_API_VISIBILITY)
#define AIVS_CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define AIVS_CJSON_PUBLIC(type) type
#endif
#endif

/* Limits how deeply nested arrays/objects can be before aivs_cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef AIVS_CJSON_NESTING_LIMIT
#define AIVS_CJSON_NESTING_LIMIT 1000
#endif

/* returns the version of aivs_cJSON as a string */
AIVS_CJSON_PUBLIC(const char*) aivs_cJSON_Version(void);

/* Supply malloc, realloc and free functions to aivs_cJSON */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_InitHooks(aivs_cJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of aivs_cJSON_Parse (with aivs_cJSON_Delete) and aivs_cJSON_Print (with stdlib free, aivs_cJSON_Hooks.free_fn, or aivs_cJSON_free as appropriate). The exception is aivs_cJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a aivs_cJSON object you can interrogate. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_Parse(const char *value);
/* Render a aivs_cJSON entity to text for transfer/storage. */
AIVS_CJSON_PUBLIC(char *) aivs_cJSON_Print(const aivs_cJSON *item);
/* Render a aivs_cJSON entity to text for transfer/storage without any formatting. */
AIVS_CJSON_PUBLIC(char *) aivs_cJSON_PrintUnformatted(const aivs_cJSON *item);
/* Render a aivs_cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
AIVS_CJSON_PUBLIC(char *) aivs_cJSON_PrintBuffered(const aivs_cJSON *item, int prebuffer, aivs_cJSON_bool fmt);
/* Render a aivs_cJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: aivs_cJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_PrintPreallocated(aivs_cJSON *item, char *buffer, const int length, const aivs_cJSON_bool format);
/* Delete a aivs_cJSON entity and all subentities. */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_Delete(aivs_cJSON *c);

/* Returns the number of items in an array (or object). */
AIVS_CJSON_PUBLIC(int) aivs_cJSON_GetArraySize(const aivs_cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_GetArrayItem(const aivs_cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_GetObjectItem(const aivs_cJSON * const object, const char * const string);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_GetObjectItemCaseSensitive(const aivs_cJSON * const object, const char * const string);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_HasObjectItem(const aivs_cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when aivs_cJSON_Parse() returns 0. 0 when aivs_cJSON_Parse() succeeds. */
AIVS_CJSON_PUBLIC(const char *) aivs_cJSON_GetErrorPtr(void);

/* These functions check the type of an item */
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsInvalid(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsFalse(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsTrue(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsBool(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsNull(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsNumber(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsString(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsArray(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsObject(const aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_IsRaw(const aivs_cJSON * const item);

/* These calls create a aivs_cJSON item of the appropriate type. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateNull(void);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateTrue(void);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateFalse(void);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateBool(aivs_cJSON_bool boolean);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateNumber(double num);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateNumberLL(long long num);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateString(const char *string);
/* raw json */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateRaw(const char *raw);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateArray(void);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateObject(void);

/* These utilities create an Array of count items. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateIntArray(const int *numbers, int count);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateFloatArray(const float *numbers, int count);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateDoubleArray(const double *numbers, int count);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_CreateStringArray(const char **strings, int count);

/* Append item to the specified array/object. */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_AddItemToArray(aivs_cJSON *array, aivs_cJSON *item);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_AddItemToObject(aivs_cJSON *object, const char *string, aivs_cJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the aivs_cJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & aivs_cJSON_StringIsConst) is zero before
 * writing to `item->string` */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_AddItemToObjectCS(aivs_cJSON *object, const char *string, aivs_cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing aivs_cJSON to a new aivs_cJSON, but don't want to corrupt your existing aivs_cJSON. */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_AddItemReferenceToArray(aivs_cJSON *array, aivs_cJSON *item);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_AddItemReferenceToObject(aivs_cJSON *object, const char *string, aivs_cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_DetachItemViaPointer(aivs_cJSON *parent, aivs_cJSON * const item);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_DetachItemFromArray(aivs_cJSON *array, int which);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_DeleteItemFromArray(aivs_cJSON *array, int which);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_DetachItemFromObject(aivs_cJSON *object, const char *string);
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_DetachItemFromObjectCaseSensitive(aivs_cJSON *object, const char *string);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_DeleteItemFromObject(aivs_cJSON *object, const char *string);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_DeleteItemFromObjectCaseSensitive(aivs_cJSON *object, const char *string);

/* Update array items. */
AIVS_CJSON_PUBLIC(void) aivs_cJSON_InsertItemInArray(aivs_cJSON *array, int which, aivs_cJSON *newitem); /* Shifts pre-existing items to the right. */
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_ReplaceItemViaPointer(aivs_cJSON * const parent, aivs_cJSON * const item, aivs_cJSON * replacement);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_ReplaceItemInArray(aivs_cJSON *array, int which, aivs_cJSON *newitem);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_ReplaceItemInObject(aivs_cJSON *object,const char *string,aivs_cJSON *newitem);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_ReplaceItemInObjectCaseSensitive(aivs_cJSON *object,const char *string,aivs_cJSON *newitem);

/* Duplicate a aivs_cJSON item */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_Duplicate(const aivs_cJSON *item, aivs_cJSON_bool recurse);
/* Duplicate will create a new, identical aivs_cJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two aivs_cJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
AIVS_CJSON_PUBLIC(aivs_cJSON_bool) aivs_cJSON_Compare(const aivs_cJSON * const a, const aivs_cJSON * const b, const aivs_cJSON_bool case_sensitive);


/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error. If not, then aivs_cJSON_GetErrorPtr() does the job. */
AIVS_CJSON_PUBLIC(aivs_cJSON *) aivs_cJSON_ParseWithOpts(const char *value, const char **return_parse_end, aivs_cJSON_bool require_null_terminated);

AIVS_CJSON_PUBLIC(void) aivs_cJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define aivs_cJSON_AddNullToObject(object,name) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateNull())
#define aivs_cJSON_AddTrueToObject(object,name) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateTrue())
#define aivs_cJSON_AddFalseToObject(object,name) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateFalse())
#define aivs_cJSON_AddBoolToObject(object,name,b) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateBool(b))
#define aivs_cJSON_AddNumberToObject(object,name,n) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateNumber(n))
#define aivs_cJSON_AddNumberLLToObject(object,name,n) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateNumberLL(n))
#define aivs_cJSON_AddStringToObject(object,name,s) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateString(s))
#define aivs_cJSON_AddRawToObject(object,name,s) aivs_cJSON_AddItemToObject(object, name, aivs_cJSON_CreateRaw(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define aivs_cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the aivs_cJSON_SetNumberValue macro */
AIVS_CJSON_PUBLIC(double) aivs_cJSON_SetNumberHelper(aivs_cJSON *object, double number);
#define aivs_cJSON_SetNumberValue(object, number) ((object != NULL) ? aivs_cJSON_SetNumberHelper(object, (double)number) : (number))

/* Macro for iterating over an array */
#define aivs_cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with aivs_cJSON_InitHooks */
AIVS_CJSON_PUBLIC(void *) aivs_cJSON_malloc(size_t size);
AIVS_CJSON_PUBLIC(void) aivs_cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif



#ifndef mibrain_cJSON__h
#define mibrain_cJSON__h



#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* mibrain_cJSON Types: */
#define mibrain_cJSON_False 0
#define mibrain_cJSON_True 1
#define mibrain_cJSON_NULL 2
#define mibrain_cJSON_Number 3
#define mibrain_cJSON_String 4
#define mibrain_cJSON_Array 5
#define mibrain_cJSON_Object 6
	
#define mibrain_cJSON_IsReference 256

/* The mibrain_cJSON structure: */
typedef struct mibrain_cJSON {
	struct mibrain_cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct mibrain_cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==mibrain_cJSON_String */
	long long valueint;				/* The item's number, if type==mibrain_cJSON_Number */
	double valuedouble;			/* The item's number, if type==mibrain_cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} mibrain_cJSON;

/* Supply a block of JSON, and this returns a mibrain_cJSON object you can interrogate. Call mibrain_cJSON_Delete when finished. */
extern mibrain_cJSON *mibrain_cJSON_Parse(const char *value);
/* Render a mibrain_cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *mibrain_cJSON_Print(mibrain_cJSON *item);
/* Render a mibrain_cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *mibrain_cJSON_PrintUnformatted(mibrain_cJSON *item);
/* Delete a mibrain_cJSON entity and all subentities. */
extern void   mibrain_cJSON_Delete(mibrain_cJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  mibrain_cJSON_GetArraySize(mibrain_cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern mibrain_cJSON *mibrain_cJSON_GetArrayItem(mibrain_cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern mibrain_cJSON *mibrain_cJSON_GetObjectItem(mibrain_cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when mibrain_cJSON_Parse() returns 0. 0 when mibrain_cJSON_Parse() succeeds. */
extern const char *mibrain_cJSON_GetErrorPtr();
	
/* These calls create a mibrain_cJSON item of the appropriate type. */
extern mibrain_cJSON *mibrain_cJSON_CreateNull();
extern mibrain_cJSON *mibrain_cJSON_CreateTrue();
extern mibrain_cJSON *mibrain_cJSON_CreateFalse();
extern mibrain_cJSON *mibrain_cJSON_CreateBool(int b);
extern mibrain_cJSON *mibrain_cJSON_CreateNumber(double num);
extern mibrain_cJSON *mibrain_cJSON_CreateString(const char *string);
extern mibrain_cJSON *mibrain_cJSON_CreateArray();
extern mibrain_cJSON *mibrain_cJSON_CreateObject();

/* These utilities create an Array of count items. */
extern mibrain_cJSON *mibrain_cJSON_CreateIntArray(int *numbers,int count);
extern mibrain_cJSON *mibrain_cJSON_CreateFloatArray(float *numbers,int count);
extern mibrain_cJSON *mibrain_cJSON_CreateDoubleArray(double *numbers,int count);
extern mibrain_cJSON *mibrain_cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void mibrain_cJSON_AddItemToArray(mibrain_cJSON *array, mibrain_cJSON *item);
extern void	mibrain_cJSON_AddItemToObject(mibrain_cJSON *object,const char *string,mibrain_cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing mibrain_cJSON to a new mibrain_cJSON, but don't want to corrupt your existing mibrain_cJSON. */
extern void mibrain_cJSON_AddItemReferenceToArray(mibrain_cJSON *array, mibrain_cJSON *item);
extern void	mibrain_cJSON_AddItemReferenceToObject(mibrain_cJSON *object,const char *string,mibrain_cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern mibrain_cJSON *mibrain_cJSON_DetachItemFromArray(mibrain_cJSON *array,int which);
extern void   mibrain_cJSON_DeleteItemFromArray(mibrain_cJSON *array,int which);
extern mibrain_cJSON *mibrain_cJSON_DetachItemFromObject(mibrain_cJSON *object,const char *string);
extern void   mibrain_cJSON_DeleteItemFromObject(mibrain_cJSON *object,const char *string);
	
/* Update array items. */
extern void mibrain_cJSON_ReplaceItemInArray(mibrain_cJSON *array,int which,mibrain_cJSON *newitem);
extern void mibrain_cJSON_ReplaceItemInObject(mibrain_cJSON *object,const char *string,mibrain_cJSON *newitem);

#define mibrain_cJSON_AddNullToObject(object,name)	mibrain_cJSON_AddItemToObject(object, name, mibrain_cJSON_CreateNull())
#define mibrain_cJSON_AddTrueToObject(object,name)	mibrain_cJSON_AddItemToObject(object, name, mibrain_cJSON_CreateTrue())
#define mibrain_cJSON_AddFalseToObject(object,name)		mibrain_cJSON_AddItemToObject(object, name, mibrain_cJSON_CreateFalse())
#define mibrain_cJSON_AddNumberToObject(object,name,n)	mibrain_cJSON_AddItemToObject(object, name, mibrain_cJSON_CreateNumber(n))
#define mibrain_cJSON_AddStringToObject(object,name,s)	mibrain_cJSON_AddItemToObject(object, name, mibrain_cJSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif

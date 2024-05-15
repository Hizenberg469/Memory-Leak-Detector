#ifndef __MLD__
#define __MLD__

#include <string.h>
#include <assert.h>

/*
* Declaration for STRUCTURE DATABASE
*/

#ifndef __STRUCT_DB__
#define __STRUCT_DB__



#define MAX_STRUCTURE_NAME_SIZE 128
#define MAX_FIELD_NAME_SIZE 128

/**Enumeration for data types**/

typedef enum {

	UINT8,
	UINT32,
	INT32,
	CHAR,
	OBJ_PTR,
	VOID_PTR, /* To identify void-pointer type */
	FLOAT,
	DOUBLE,
	OBJ_STRUCT

}data_type_t;

typedef enum {

	MLD_FALSE,
	MLD_TRUE

} mld_boolean_t;

/**Enumeration for data types**/



#define FIELD_OFFSET( struct_name, field_name ) \
	(unsigned int)&(((struct_name* )0)->field_name)

#define FIELD_SIZE( struct_name, field_name ) \
	sizeof(((struct_name* )0)->field_name)


/******Struture to store the information of one field of a C structure******/

typedef struct _field_info_ {

	char fname[MAX_FIELD_NAME_SIZE];				//Name of the field
	data_type_t dtype;								//Data type registered
	unsigned int size;								//Size of the field
	unsigned int offset;							//Offset of field in the structure object

	// Below field is meaningful only if dtype = OBJ_PTR, Or OBJ_STRUCT

	char nested_str_name[MAX_STRUCTURE_NAME_SIZE];
}field_info_t;

/******Struture to store the information of one field of a C structure******/



/******Structure to store the information of one C structure which could have 'n_fields' fields******/

typedef struct _struct_db_rec_ {

	struct _struct_db_rec_* next;							//Pointer to next structure in db
	char struct_name[MAX_STRUCTURE_NAME_SIZE];		//Name of Structure variable
	unsigned int ds_size;							//Size of structure object
	unsigned int n_fields;							//No. of fields in this structure
	field_info_t* fields;							//Pointer to array of fields

} struct_db_rec_t;

/******Structure to store the information of one C structure which could have 'n_fields' fields******/


/*****Structure Data base Definition******/

typedef struct _struct_db_ {

	struct_db_rec_t* head;							//Head of the Database
	unsigned int count;								//No. of records in Database

} struct_db_t;

/*****Structure Data base Definition******/



/**********Printing functions**********/

void
print_structure_rec(struct_db_rec_t* struct_rec);

void
print_structure_db(struct_db_t* struct_db);

/**********Printing functions**********/


/******Function to add the structure record to structure db******/

/*return 0 on success, -1 on failure from some reason*/
int
add_structure_to_struct_db(struct_db_t* struct_db, struct_db_rec_t* struct_rec);

/******Function to add the structure record to structure db******/

/******Function to lookup the structure record in structure db******/

/*return pointer of struct_db_rec_t on success, NULL on failure from some reason*/
struct_db_rec_t*
struct_db_look_up(struct_db_t* struct_db, char* struct_name);

/******Function to lookup the structure record in structure db******/


/*********Structure Registration helpint API***********/

/**#fld_name is used to convert the fld_name to string "fld_name" if used in #define preprocessor**/

#define FIELD_INFO( struct_name, fld_name, dtype, nested_struct_name )  \
	{#fld_name, dtype, FIELD_SIZE(struct_name, fld_name),				\
		FIELD_OFFSET(struct_name, fld_name), #nested_struct_name }

#define REG_STRUCT(struct_db, st_name, fields_arr)		\
	do{													\
		struct_db_rec_t* rec = calloc(1, sizeof(struct_db_rec_t)); \
		rec->next = NULL;								\
		strncpy(rec->struct_name, #st_name, MAX_STRUCTURE_NAME_SIZE); \
		rec->ds_size = sizeof(st_name);					\
		rec->n_fields = sizeof(fields_arr)/sizeof(field_info_t); \
		rec->fields = fields_arr;						\
		if( add_structure_to_struct_db(struct_db, rec)){ \
			assert(0);									\
		}												\
	}while(0);


/*********Structure Registration helpint API***********/

#endif

/*
* Declaration for STRUCTURE DATABASE
*/



#ifndef __OBJECT_DB__
#define __OJBECT_DB__

/*
* Declaration for OBJECT DATABASE
*/

typedef struct _object_db_rec_ object_db_rec_t;

struct _object_db_rec_ {
	
	object_db_rec_t* next;							//Pointer to next object registered in OJBECT DB
	void* ptr;										//Pointer to object which it registered
	unsigned int units;								//No. of objects created
	struct_db_rec_t* struct_rec;					//Structure type it belongs to...
	mld_boolean_t is_visited;						//Used for Graph traversal
	mld_boolean_t is_root;							//Is this object is Root object

};

typedef struct _object_db_ {

	struct_db_t* struct_db;							//Pointer to OBJECT DATABASE
	object_db_rec_t* head;							//Head of the DB
	unsigned int count;								//No. of registered OBJECT in DB

} object_db_t;


/****Printing functions for dumping object db****/

void
print_object_rec(object_db_rec_t* obj_rec, int i);

void
print_object_db(object_db_t* object_db);

/****Printing functions for dumping object db****/


/*****API to calloc the object******/


void*
xcalloc(object_db_t* object_db, char* struct_name, int units);



/*****API to calloc the object******/


/*****API to calloc the object******/

void
xfree(object_db_t* object_db, void* ptr);


/*****API to calloc the object******/



/*****APIs to register root objects******/

void
mld_register_global_object_as_root(object_db_t* object_db,
	void* objptr,
	char* struct_name,
	unsigned int units);

void
mld_set_dynamic_object_as_root(object_db_t* object_db, void* obj_ptr);

/*****APIs to register root objects******/


/*********************************************APIs' to implement MLD algorithm*************************************************/

void
report_leaked_objects(object_db_t* object_db);

/*********************************************APIs' to implement MLD algorithm*************************************************/


/********Adding support for primitive data type********/

void
run_mld_algorithm(object_db_t* object_db);

void
mld_init_primitive_data_types_support(struct_db_t* struct_db);

/********Adding support for primitive data type********/

/*
* Declaration for OBJECT DATABASE
*/

#endif




#endif
#include <stdio.h>
#include <stdlib.h>
#include "mld.h"
#include "css.h"
#include <assert.h>
#include <memory.h>
#include <limits.h>

char* DATA_TYPE[] = { "UINT8", "UINT32", "INT32",
					"CHAR", "OBJ_PTR", "VOID_PTR", "FLOAT",
					"DOUBLE", "OBJ_STRUCT" };

/**********Printing functions**********/

void
print_structure_rec(struct_db_rec_t* struct_rec) {

	if (!struct_rec)
		return;

	int j = 0;

	field_info_t* field = NULL; 
	printf(ANSI_COLOR_CYAN "|--------------------------------------------------------------|\n" ANSI_COLOR_RESET);
	printf(ANSI_COLOR_YELLOW "| %-20s   | size = %-8d | #flds = %-3d       |\n" ANSI_COLOR_RESET,
		struct_rec->struct_name, struct_rec->ds_size, struct_rec->n_fields);

	printf(ANSI_COLOR_CYAN "|--------------------------------------------------------------|--------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
	
	for (j = 0; j < struct_rec->n_fields; j++) {
		field = &struct_rec->fields[j];
		printf("   %-20s  |", "");
		printf("%-3d %-20s | dtype = %-15s | size = %-5d | offset = %-6d | nstructname = %-20s    |\n",
			j, field->fname, DATA_TYPE[field->dtype], field->size, field->offset, field->nested_str_name);
		printf("  %-20s   |", "");
		printf(ANSI_COLOR_CYAN "----------------------------------------------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
	}
}

void
print_structure_db(struct_db_t* struct_db) {
	
	if (!struct_db)
		return;
	printf("printing STRUCTURE DATABASE\n");

	int i = 0;
	struct_db_rec_t* struct_rec = NULL;
	struct_rec = struct_db->head;
	printf("No of Structures Registered = %d\n", struct_db->count);
	while (struct_rec) {

		/****%p format specifier is used to print value of void* pointer in hexadecimal format(prints the address value)****/
		printf("Structure No : %d (%p)\n", i++, struct_rec);
		print_structure_rec(struct_rec);
		struct_rec = struct_rec->next;
	}
}

/**********Printing functions**********/


/******Function to add the structure record to structure db******/

/*return 0 on success, -1 on failure from some reason*/
int
add_structure_to_struct_db(struct_db_t* struct_db, struct_db_rec_t* struct_rec) {

	struct_db_rec_t* head = struct_db->head;

	if (!head) {
		struct_db->head = struct_rec;
		struct_rec->next = NULL;
		struct_db->count++;
		return 0;
	}

	struct_rec->next = head;
	struct_db->head = struct_rec;
	struct_db->count++;
	return 0;
}

/******Function to add the structure record to structure db******/



/******Function to lookup the structure record in structure db******/

/*return pointer of struct_db_rec_t on success, NULL on failure from some reason*/
struct_db_rec_t*
struct_db_look_up(struct_db_t* struct_db, char* struct_name) {

	if (!struct_db || !struct_name)
		return NULL;

	struct_db_rec_t* struct_rec = struct_db->head;

	if (!struct_rec)
		return NULL;
	
	unsigned int sz_db = struct_db->count;

	struct_db_rec_t* desired_rec = NULL;

	int i = 0;
	while (i < sz_db) {

		if (!strncmp(struct_rec->struct_name, struct_name, MAX_STRUCTURE_NAME_SIZE)) {
			desired_rec = struct_rec;
			break;
		}

		struct_rec = struct_rec->next;
		i++;
	}

	return desired_rec;
}

/******Function to lookup the structure record in structure db******/





/******Function to lookup the object record in object db******/


static object_db_rec_t*
object_db_look_up(object_db_t* object_db,
	void* ptr) {

	object_db_rec_t* head = object_db->head;
	if (!head)
		return NULL;

	unsigned int sz_db = object_db->count;

	object_db_rec_t* desired_obj = NULL;
	int i = 0;
	while (i < sz_db) {

		if (head->ptr == ptr) {
			desired_obj = head;
			break;
		}

		head = head->next;
		i++;

	}

	return desired_obj;
}
/******Function to lookup the object record in object db******/



/******Function to add object to OBJECT DB*******/

static void
add_object_to_object_db(object_db_t* object_db,
	void* ptr,
	int units,
	struct_db_rec_t* struct_rec,
	mld_boolean_t is_root) {

	object_db_rec_t* obj_rec = object_db_look_up(object_db, ptr);

	/**Don't add some object twice**/
	assert(!obj_rec);

	obj_rec = calloc(1, sizeof(object_db_rec_t));

	obj_rec->next = NULL;
	obj_rec->ptr = ptr;
	obj_rec->units = units;
	obj_rec->struct_rec = struct_rec;
	obj_rec->is_visited = MLD_FALSE;
	obj_rec->is_root = is_root;

	object_db_rec_t* head = object_db->head;

	if (!head) {
		object_db->head = obj_rec;
		obj_rec->next = NULL;
		object_db->count++;
		return;
	}


	obj_rec->next = head;
	object_db->head = obj_rec;
	object_db->count++;

}

void*
xcalloc(object_db_t* object_db,
	char* struct_name,
	int units) {

	struct_db_rec_t* struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
	assert(struct_rec);
	void* ptr = calloc(units, struct_rec->ds_size);
	add_object_to_object_db(object_db, ptr, units,
		struct_rec, MLD_FALSE);
	/*xcalloc by default set the object as non-root*/
	return ptr;

}

/******Function to add object to OBJECT DB*******/



/******Function to delete object record from object db******/


static void
delete_object_record_from_object_db(object_db_t* object_db,
	object_db_rec_t* object_rec) {

	assert(object_rec);

	object_db_rec_t* head = object_db->head;
	if (head == object_rec) {
		object_db->head = object_rec->next;
		free(object_rec);
		return;
	}

	object_db_rec_t* prev = head;
	head = head->next;

	while (head) {
		if (head != object_rec) {
			prev = head;
			head = head->next;
			continue;
		}

		prev->next = head->next;
		head->next = NULL;
		free(head);
		return;
	}

}


void
xfree(object_db_t* object_db, void* ptr) {

	if (!ptr)
		return;

	object_db_rec_t* object_rec =
		object_db_look_up(object_db, ptr);

	assert(object_rec);
	assert(object_rec->ptr);
	free(object_rec->ptr);
	object_rec->ptr = NULL;
	
	/*Delete object record from object db*/
	delete_object_record_from_object_db(object_db, object_rec);

}

/******Function to delete object record from object db******/





/****Printing functions for dumping object db****/

void
print_object_rec(object_db_rec_t* obj_rec, int i) {

	if (!obj_rec)
		return;

	printf(ANSI_COLOR_MAGENTA "|-------------------------------------------------------------------------------------------------------------|\n"ANSI_COLOR_RESET);
	printf(ANSI_COLOR_YELLOW "|%-3d ptr = %-10p | next = %-10p | units = %-4d | struct_name = %-10s | is_root = %s |\n"ANSI_COLOR_RESET,
		i, obj_rec->ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name, obj_rec->is_root == MLD_TRUE? "TRUE " : "FALSE");
	printf(ANSI_COLOR_MAGENTA "|-------------------------------------------------------------------------------------------------------------|\n"ANSI_COLOR_RESET);

}

void
print_object_db(object_db_t* object_db) {


	object_db_rec_t* object_rec = object_db->head;
	unsigned int i = 0;
	printf(ANSI_COLOR_CYAN "Printing OBJECT DATABASE\n" ANSI_COLOR_RESET);
	for (; object_rec; object_rec = object_rec->next) {
		print_object_rec(object_rec, i++);
	}

}

/****Printing functions for dumping object db****/



/*****APIs to register root objects******/

/***

This API is used to register global object of certain as root object, which are global in scope.
Object is not initialized by xcalloc() function.

***/

void
mld_register_global_object_as_root(object_db_t* object_db,
	void* objptr,
	char* struct_name,
	unsigned int units) {

	struct_db_rec_t* req_struct_rec = struct_db_look_up(object_db->struct_db, struct_name);

	assert(req_struct_rec);

	/*Create a new object record and add it to object database with it being a root object*/
	add_object_to_object_db(object_db, objptr, units, req_struct_rec, MLD_TRUE);

}

void
mld_set_dynamic_object_as_root(object_db_t* object_db, void* obj_ptr) {

	
	object_db_rec_t* req_obj_rec = object_db_look_up(object_db, obj_ptr);

	assert(req_obj_rec != NULL);


	req_obj_rec->is_root = MLD_TRUE;

}

/*****APIs to register root objects******/



/************************************************APIs' to implement MLD algorithm***************************************************/


static object_db_rec_t*
get_next_root_object(object_db_t* object_db, object_db_rec_t* last_root_object) {

	object_db_rec_t* cur_obj_rec = last_root_object ? last_root_object->next : object_db->head;

	//assert(cur_obj_rec);

	while (cur_obj_rec) {

		if (cur_obj_rec->is_root == MLD_TRUE)
			return cur_obj_rec;

		cur_obj_rec = cur_obj_rec->next;
	}

	return NULL;
}


static void
init_mld_algorithm(object_db_t* object_db) {

	object_db_rec_t* cur_obj = object_db->head;

	while (cur_obj) {
		cur_obj->is_visited = MLD_FALSE;
		cur_obj = cur_obj->next;
	}
}

//DFS algorithm to explore the Graphically oriented object set...
static void
mld_explore_object_recursively(object_db_t* object_db,
	object_db_rec_t* parent_obj_rec) {

	unsigned int i, n_fields;
	char* parent_obj_ptr = NULL,
		* child_obj_offset = NULL;
	void* child_object_address = NULL;
	field_info_t* field_info = NULL;

	object_db_rec_t* child_object_rec = NULL;
	struct_db_rec_t* parent_struct_rec = parent_obj_rec->struct_rec;

	/*Parent object must have already visited*/

	assert(parent_obj_rec->is_visited == MLD_TRUE);

	if (parent_struct_rec->n_fields == 0) {
		return;
	}

	for (i = 0; i < parent_obj_rec->units; i++) {

		parent_obj_ptr = (char*)(parent_obj_rec->ptr) + (i * parent_struct_rec->ds_size);

		for (n_fields = 0; n_fields < parent_struct_rec->n_fields; n_fields++) {

			field_info = &parent_struct_rec->fields[n_fields];

			/*Only need to handle void* and obj_ptr*/
			switch (field_info->dtype) {
			case UINT8:
			case UINT32:
			case INT32:
			case CHAR:
			case FLOAT:
			case DOUBLE:
			case OBJ_STRUCT:
				break;
			case VOID_PTR:
			case OBJ_PTR:
			default:
				

				/*child_obj_offset is the memory location inside parent object
				where address of next level object is stored*/

				child_obj_offset = parent_obj_ptr + field_info->offset;
				memcpy(&child_object_address, child_obj_offset, sizeof(void*));

				/*child_obj_address now stores the address of the next object in the graph.
				It could be NULL, Handle that as well*/

				if (!child_object_address)
					continue;

				child_object_rec = object_db_look_up(object_db, child_object_address);

				assert(child_object_rec);

				/*Since it's a valid child object of the given parent object,
				explore it with leisure for memory leak*/
				if (child_object_rec->is_visited == MLD_FALSE) {
					child_object_rec->is_visited = MLD_TRUE;
					if (field_info->dtype != VOID_PTR)/*Explore next object only when it is not a VOID_PTR*/
						mld_explore_object_recursively(object_db, child_object_rec);

				}
				else {
					continue; /*Do nothing, explore next child object*/
				}
			}
		}
	}
}


/** Public API to run MLD Algorithm **/
void
run_mld_algorithm(object_db_t* object_db) {

	//Necessary initialisation...
	init_mld_algorithm(object_db);

	//Get the first root object...
	object_db_rec_t* root_obj = get_next_root_object(object_db, NULL);

	while (root_obj) {
		if (root_obj->is_visited == MLD_TRUE) {
			//Exploration from this node object is already done...
			root_obj = get_next_root_object(object_db, root_obj);
			continue;
		}

		root_obj->is_visited = MLD_TRUE;

		mld_explore_object_recursively(object_db, root_obj);

		root_obj = get_next_root_object(object_db, root_obj);
	}
}

static void
mld_dump_object_rec_detail(object_db_rec_t* obj_rec) {

	int n_fields = obj_rec->struct_rec->n_fields;

	field_info_t* field = NULL;

	int units = obj_rec->units, obj_index = 0,
		field_index = 0;

	for (; obj_index < units; obj_index++) {
		char* current_object_ptr = (char*)(obj_rec->ptr) + 
			(obj_index * obj_rec->struct_rec->ds_size);

		for (field_index = 0; field_index < n_fields; field_index++) {
			
			field = &obj_rec->struct_rec->fields[field_index];

			switch (field->dtype) {
			case UINT8:
			case INT32:
			case UINT32:
				printf("%s[%d]->%s = %d\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(int*)(current_object_ptr + field->offset));
				break;
			case CHAR:
				printf("%s[%d]->%s = %s\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, (char*)(current_object_ptr + field->offset));
				break;
			case FLOAT:
				printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(float*)(current_object_ptr + field->offset));
				break;
			case DOUBLE:
				printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(double*)(current_object_ptr + field->offset));
				break;
			case OBJ_PTR:
				printf("%s[%d]->%s = %p\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, (void*)*(int*)(current_object_ptr + field->offset));
				break;
			case OBJ_STRUCT:
				/*Later*/
				break;
			default:
				break;
			}
		}
	}
}

void
report_leaked_objects(object_db_t* object_db) {

	int i = 0;
	object_db_rec_t* head;

	printf("Dumping Leaked Objects\n");

	for (head = object_db->head; head; head = head->next) {
		if (head->is_visited == MLD_FALSE) {
			print_object_rec(head, i++);
			mld_dump_object_rec_detail(head);
			printf("\n\n");
		}
	}
}



/************************************************APIs' to implement MLD algorithm***************************************************/




/********Adding support for primitive data type********/

void
mld_init_primitive_data_types_support(struct_db_t* struct_db) {

	REG_STRUCT(struct_db, int, 0);
	REG_STRUCT(struct_db, float, 0);
	REG_STRUCT(struct_db, double, 0);

}

/********Adding support for primitive data type********/

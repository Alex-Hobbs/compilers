#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H
#include "nodes.h"
#include "common.h"

typedef struct env_binding
{
  char					   *name;
  TOKEN					   *value;
  struct env_binding	   *next;
} ENVIRONMENT_BINDING;

typedef struct env_frame
{
	NODE						*body;
	ENVIRONMENT_BINDING			*bindings;
	NODE						*declaration;
	char						*name;
	struct env_frame			*next;
	char						*return_type;
	int							return_value;
} ENVIRONMENT_FRAME;

typedef struct runtime_values
{
	int							value;
	struct runtime_values		*next;
} RUNTIME_VALUES;

ENVIRONMENT_BINDING* previous_binding = NULL;

TOKEN* 					lookup_variable(ENVIRONMENT_BINDING*, char*);
ENVIRONMENT_FRAME* 		extend_environment( ENVIRONMENT_FRAME*, ENVIRONMENT_BINDING* );
ENVIRONMENT_BINDING* 	define_variable_with_value( ENVIRONMENT_BINDING*, char*, TOKEN* );
ENVIRONMENT_FRAME* 		add_bindings_to_environment( ENVIRONMENT_FRAME*, ENVIRONMENT_BINDING* );
ENVIRONMENT_FRAME* 		update_environment_with_metadata( ENVIRONMENT_FRAME*, char*, char* );
NODE* 					get_body_of_function( ENVIRONMENT_FRAME*, char* );
NODE* 					get_declaration_of_function( ENVIRONMENT_FRAME*, char* );
int 					get_value_from_tree( ENVIRONMENT_BINDING*, NODE* );
ENVIRONMENT_FRAME*		set_environment_return_value( ENVIRONMENT_FRAME*, int );
ENVIRONMENT_FRAME* 		store_function( ENVIRONMENT_FRAME*, NODE*, NODE* );
ENVIRONMENT_FRAME* 		setup_environment( ENVIRONMENT_FRAME* );
ENVIRONMENT_FRAME* 		parse_environment( ENVIRONMENT_FRAME*, NODE* );

#endif

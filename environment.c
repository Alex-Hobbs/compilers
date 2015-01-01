#include <stdio.h>
#include <stdlib.h>
#include "environment.h"
#include "token.h"
#include "C.tab.h"

/**
* Lookup variable value for a given environment/frame
*/
TOKEN* lookup_variable( ENVIRONMENT_BINDING* node, char* variable_name )
{
	if( node == NULL )
		return NULL;

	// Do something about scanning through environments?
	printf( "Looking for %s, found %s\n", variable_name, node->name );


	if ( strcmp( node->name, variable_name ) != 0 )
		return lookup_variable( node->next, variable_name );

	return node->value;
}

/**
 * Extend an environment.
 *
 * Returns new environment with a name frame storing both variables and values inside this frame
 */
ENVIRONMENT_FRAME* extend_environment( ENVIRONMENT_FRAME* base_environment, ENVIRONMENT_BINDING* variables )
{
	ENVIRONMENT_FRAME *frame = (ENVIRONMENT_FRAME*)malloc( sizeof( ENVIRONMENT_FRAME ) );

	frame->bindings = variables;
	frame->next = base_environment;

	return frame;
}

ENVIRONMENT_FRAME* add_bindings_to_environment( ENVIRONMENT_FRAME* frame, ENVIRONMENT_BINDING* variables )
{
	frame->bindings = variables;

	return frame;
}

ENVIRONMENT_FRAME* update_environment_with_metadata( ENVIRONMENT_FRAME* frame, char* function_name, char* return_type )
{
	frame->name 		= function_name;
	frame->return_type 	= return_type;

	printf( "Environment extended with new environment of: %s %s\n", return_type, function_name );

	return frame;
}

/**
 * Define a variable and a value in a given environment
 */
ENVIRONMENT_BINDING* define_variable_with_value(
	ENVIRONMENT_FRAME *environment, 
	ENVIRONMENT_BINDING *base_binding, 
	char* variable_name, 
	TOKEN *value
)
{
	ENVIRONMENT_BINDING *binding = (ENVIRONMENT_BINDING*)malloc( sizeof( ENVIRONMENT_BINDING ) );

	binding->name  = variable_name;
	binding->value = value;
	binding->next  = base_binding;

	printf( "Variable %s added with value %d\n", variable_name, value->value );

	return binding;
}

/**
 * Update a variable value for a variable which is already set within a given environment
 */
ENVIRONMENT_BINDING* update_variable_with_value(
	ENVIRONMENT_FRAME *environment,
	ENVIRONMENT_BINDING *binding,
	char* variable_name,
	TOKEN *value
)
{
	if (binding == NULL)
		return NULL;

	if (binding->name != variable_name)
		return update_variable_with_value(environment, binding->next, variable_name, value);

	binding->value = value;
	return binding;
}
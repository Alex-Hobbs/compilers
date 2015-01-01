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
	// Do something about scanning through environments?

	if( node == NULL )
		return NULL;

	if ( node->name != variable_name )
		return get_value_from_variable( node->next, variable_name );

	return node->value;
}

TOKEN* get_value_from_variable( ENVIRONMENT_BINDING* node, char* variable_name )
{
	return NULL;
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

	printf( "Environment extended\n" );

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
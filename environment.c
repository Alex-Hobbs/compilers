#include <stdio.h>
#include <stdlib.h>
#include "environment.h"
#include "C.tab.h"

ENVIRONMENT_BINDING *previous_binding;

ENVIRONMENT_FRAME* add_bindings_to_environment( ENVIRONMENT_FRAME* environment, ENVIRONMENT_BINDING* variables )
{
	environment->bindings = variables;
	return environment;
}

/**
 * Define a variable and a value in a given environment
 */
ENVIRONMENT_BINDING* define_variable_with_value( ENVIRONMENT_BINDING* next_variable, char* variable_name, TOKEN* value )
{
	ENVIRONMENT_BINDING *binding = (ENVIRONMENT_BINDING*) malloc( sizeof( ENVIRONMENT_BINDING ) );

	binding->name  = variable_name;
	binding->value = value;
	binding->next  = next_variable;

	return binding;
}

/**
 * Extend an environment.
 *
 * Returns new environment with a name frame storing both variables and values inside this frame
 */
ENVIRONMENT_FRAME* extend_environment( ENVIRONMENT_FRAME* base_environment, ENVIRONMENT_BINDING* variables )
{
	ENVIRONMENT_FRAME *frame = (ENVIRONMENT_FRAME*) malloc( sizeof( ENVIRONMENT_FRAME ) );

	frame->bindings = variables;
	frame->next = base_environment;

	return frame;
}

NODE* get_body_of_function( ENVIRONMENT_FRAME* frame, char* function_name )
{
	if ( frame == NULL )
		return NULL;

	if ( strcmp( frame->name, function_name ) != 0 )
		return get_body_of_function( frame->next, function_name );

	return frame->body;
}

NODE* get_declaration_of_function( ENVIRONMENT_FRAME* frame, char* function_name )
{
	if ( frame == NULL )
		return NULL;

	if ( strcmp( frame->name, function_name ) != 0 )
		return get_declaration_of_function( frame->next, function_name );

	return frame->declaration;
}

int get_value_from_tree( ENVIRONMENT_BINDING *binding, NODE *value )
{
    int test_value;
  
    test_value = get_int_from_leaf( value );

    // If the value we get back is MAX_INTEGER, then we are not a number
    // therefore lookup variable value.
    if ( test_value == MAX_INTEGER )
    {
    	TOKEN* lookup_var = lookup_variable( binding, get_leaf( value ) );

    	if ( lookup_var != NULL )
        {
        	test_value = get_int_from_token( lookup_var );
        }

        // We're still a letter? We cannot apply arithmetic to a number, error out.
        if ( test_value == MAX_INTEGER )
        {
            return 0;
        }
    }

    return (int) test_value;
}

/**
* Lookup variable value for a given environment/frame
*/
TOKEN* lookup_variable( ENVIRONMENT_BINDING* node, char* variable_name )
{
	if( node == NULL ) return NULL;

	if ( strcmp( node->name, variable_name ) == 0 )
		return (TOKEN *)node->value;

	if ( node->next != NULL )
		return lookup_variable( node->next, variable_name );

	TOKEN* passed_value = (TOKEN *)malloc( sizeof( TOKEN ) );
	passed_value = new_token(260); // String literal
	passed_value->value = atoi(variable_name);
	passed_value->lexeme = variable_name;

	return passed_value;
}

ENVIRONMENT_FRAME* parse_environment( ENVIRONMENT_FRAME* current_frame, NODE* tree )
{
    if (tree==NULL) return current_frame;

    if (tree->type == LEAF)
    {
        return current_frame;
    }
    else
    {
        char *function_name = NULL;
        ENVIRONMENT_FRAME *new_frame = NULL;
        NODE *next_tree = NULL;

        switch( tree->type )
        {
            // Entered a new function
            case ENVIRONMENT:
                new_frame = extend_environment( current_frame, NULL );
                new_frame = store_function( new_frame, tree->left, tree->right );

                previous_binding = NULL;
                current_frame = new_frame;
                break;
            
            case RETURN:
                if ( current_frame->return_value && current_frame->next != NULL )
                {
                    current_frame->next->return_value = process_return(
                            current_frame,
                            current_frame->next->body->right,
                            current_frame->next->name,
                            current_frame->next->declaration,
                            current_frame->body,
                            current_frame->next->body->right->left->right
                    );

                    return current_frame;
                }

                current_frame->return_value = process_return( current_frame, tree, NULL, NULL, NULL, NULL );
                return current_frame;

            case DECLARATION:
                current_frame = process_function( current_frame, tree->left, tree->right );
                break;

            // Found a list of variables
            case TILDA:
                process_variables( current_frame, tree );
                current_frame = add_bindings_to_environment( current_frame, previous_binding );
                break;

            case IF:
                current_frame = process_conditional( current_frame, tree, tree->left->type );
                return current_frame;
        }
    }

    //  Return clause
    if( current_frame->return_value )
        return current_frame;

    current_frame = parse_environment( current_frame, tree->left );
    current_frame = parse_environment( current_frame, tree->right );
    return current_frame;
}

ENVIRONMENT_FRAME* set_environment_return_value( ENVIRONMENT_FRAME* environment, int value )
{
	environment->return_value = value;
	return environment;
}

ENVIRONMENT_FRAME* setup_new_environment( ENVIRONMENT_FRAME* neighbour )
{
    ENVIRONMENT_FRAME *environment = (ENVIRONMENT_FRAME*) malloc( sizeof( ENVIRONMENT_FRAME ) );
    environment->next = neighbour;
    return environment;
}

ENVIRONMENT_FRAME* store_function( ENVIRONMENT_FRAME* frame, NODE* declaration, NODE* body )
{
	frame->body = body;
	frame->declaration = declaration;
}

ENVIRONMENT_FRAME* update_environment_with_metadata( ENVIRONMENT_FRAME* frame, char* function_name, char* return_type )
{
	frame->name 		= function_name;
	frame->return_type 	= return_type;

	return frame;
}
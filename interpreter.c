#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

NODE*               previous_node;

RUNTIME_VALUES* process_apply_params( ENVIRONMENT_FRAME* frame, NODE* tree, RUNTIME_VALUES *valueList )
{
    if ( tree == NULL ) return valueList;

    if ( tree->type != LEAF )
    {
        valueList = process_apply_params( frame, tree->left, valueList );
        valueList = process_apply_params( frame, tree->right, valueList );
    }
    else
    {
        int value = get_int_from_leaf( tree->left );
        
        RUNTIME_VALUES *valuePtr = (RUNTIME_VALUES*)malloc( sizeof( RUNTIME_VALUES ) );
        valuePtr->next = valueList;
        valuePtr->value = value;
        return valuePtr;
    }
}

ENVIRONMENT_FRAME* process_apply( ENVIRONMENT_FRAME* frame, NODE *declaration, NODE *body, char *function_name, NODE *parameters )
{
    RUNTIME_VALUES *values = process_apply_params( frame, parameters, NULL );

    // Setup tmp environment
    ENVIRONMENT_FRAME *tmpEnv = setup_new_environment( NULL );
    tmpEnv->bindings    = frame->bindings;
    tmpEnv->declaration = declaration;
    tmpEnv->body        = body;
    tmpEnv->name        = function_name; 
    tmpEnv->next        = frame->next;

    ENVIRONMENT_BINDING *bindings = frame->bindings;
    ENVIRONMENT_BINDING *firstBinding = bindings;

    while( values != NULL )
    {
        TOKEN *newValue = new_token( CONSTANT );
        newValue->value = values->value;
        bindings->value = newValue;

        bindings = bindings->next;
        values = values->next;
    }

    // Rewrite our bindings
    tmpEnv->bindings = firstBinding;
    //print_tree0( body, 50 );
    frame = parse_environment( tmpEnv, body );
    
    return frame;
}

int process_leaf( ENVIRONMENT_FRAME *frame, NODE *leaf )
{
    int program_value;
    char* leaf_name;

    if( get_int_from_leaf( leaf ) != 0 )
    {
        program_value = get_int_from_leaf( leaf );
    }
    else
    {
        leaf_name = get_leaf( leaf );
        program_value = get_int_from_token( lookup_variable( frame->bindings, leaf_name ) );
    }

    return program_value;
}



int process_return( ENVIRONMENT_FRAME *frame, NODE *tree, char *function_name, NODE *declaration, NODE *body, NODE *parameters )
{
    char* left_variable_name;
    char* right_variable_name;
    int right_int;
    int left_int;
    int program_value;
    TOKEN* left;
    TOKEN* right;

    switch( tree->left->type )
    {
        case APPLY:
            if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
            {
                function_name = get_leaf( tree->left->left->left );
                declaration   = get_declaration_of_function( frame, function_name );
                body          = get_body_of_function( frame, function_name );
                parameters    = tree->left->right;
            }

           // print_tree0( body, 100 );

            frame = process_apply( frame, declaration, body, function_name, parameters );
            program_value = frame->return_value;
            //printf( "Program value = %d\n", program_value );
            frame = frame->next;
            break;


        case IF:
            if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
            {
                function_name = get_leaf( tree->left->left->left );
                declaration   = get_declaration_of_function( frame, function_name );
                body          = get_body_of_function( frame, function_name );
                parameters    = tree->left->right;
            }

            print_tree0( body, 15 );

            frame = process_conditional( frame, body, body>left->type );
            program_value = frame->return_value;
            break;

        case '+':
          if( tree->left->left->type == APPLY )
          {
                if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
                {
                    function_name = get_leaf( tree->left->left->left->left );
                    declaration   = get_declaration_of_function( frame, function_name );
                    body          = get_body_of_function( frame, function_name );
                    parameters    = tree->left->left->right;
                }

                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, declaration, body, function_name, parameters );

                if ( right_int != 0 )
                {
                    //printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
                    program_value = right_int + frame->return_value;
                }
                else
                {
                    right_variable_name = get_leaf( tree->left->right->left );
                    right = lookup_variable( frame->bindings, right_variable_name );
                    program_value = get_int_from_token( right ) + frame->return_value;
                }

                frame = frame->next;
          }
          else
          {
                left_variable_name = get_leaf( tree->left->left->left );
                right_variable_name = get_leaf( tree->left->right->left );
                left = lookup_variable( frame->bindings, left_variable_name );
                right = lookup_variable( frame->bindings, right_variable_name );
                program_value = get_int_from_token( left ) + get_int_from_token( right );
          }
          break;

        case '-':
          if( tree->left->left->type == APPLY )
          {
                if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
                {
                    function_name = get_leaf( tree->left->left->left->left );
                    declaration   = get_declaration_of_function( frame, function_name );
                    body          = get_body_of_function( frame, function_name );
                    parameters    = tree->left->left->right;
                }

                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, declaration, body, function_name, parameters );

                if ( right_int != 0 )
                {
                    //printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
                    program_value = frame->return_value - right_int;
                }
                else
                {
                    right_variable_name = get_leaf( tree->left->right->left );
                    right = lookup_variable( frame->bindings, right_variable_name );
                    program_value = frame->return_value - get_int_from_token( right );
                }

                frame = frame->next;
          }
          else
          {
                left_variable_name = get_leaf( tree->left->left->left );
                right_variable_name = get_leaf( tree->left->right->left );
                left = lookup_variable( frame->bindings, left_variable_name );
                right = lookup_variable( frame->bindings, right_variable_name );
                program_value = get_int_from_token( left ) - get_int_from_token( right );
          }
          break;
          
        case 42:
          if( tree->left->left->type == APPLY )
          {
                if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
                {
                    function_name = get_leaf( tree->left->left->left->left );
                    declaration   = get_declaration_of_function( frame, function_name );
                    body          = get_body_of_function( frame, function_name );
                    parameters    = tree->left->left->right;
                } 

                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, declaration, body, function_name, parameters );

                if ( right_int != 0 )
                {
                    //printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
                    program_value = frame->return_value * right_int;
                }
                else
                {
                    right_variable_name = get_leaf( tree->left->right->left );
                    right = lookup_variable( frame->bindings, right_variable_name );
                    program_value = frame->return_value * get_int_from_token( right );
                }

                frame = frame->next;
          }
          else
          {
                left_variable_name = get_leaf( tree->left->left->left );
                right_variable_name = get_leaf( tree->left->right->left );
                left = lookup_variable( frame->bindings, left_variable_name );
                right = lookup_variable( frame->bindings, right_variable_name );
                program_value = get_int_from_token( left ) * get_int_from_token( right );
          }
          break;
          
        case '/':
          if( tree->left->left->type == APPLY )
          {
                if ( function_name == NULL || declaration == NULL || body == NULL || parameters == NULL )
                {
                    function_name = get_leaf( tree->left->left->left->left );
                    declaration   = get_declaration_of_function( frame, function_name );
                    body          = get_body_of_function( frame, function_name );
                    parameters    = tree->left->left->right;
                }

                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, declaration, body, function_name, parameters );

                if ( right_int != 0 )
                {
                    //printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
                    program_value = frame->return_value / right_int;
                }
                else
                {
                    right_variable_name = get_leaf( tree->left->right->left );
                    right = lookup_variable( frame->bindings, right_variable_name );
                    program_value = frame->return_value / get_int_from_token( right );
                }

                frame = frame->next;
          }

          else
          {
                left_variable_name = get_leaf( tree->left->left->left );
                right_variable_name = get_leaf( tree->left->right->left );
                left = lookup_variable( frame->bindings, left_variable_name );
                right = lookup_variable( frame->bindings, right_variable_name );
                program_value = get_int_from_token( left ) / get_int_from_token( right );
          }
          break;

        case LEAF:
            //printf( "Leaf = yes = %s\n", tree->left->left );
            program_value = process_leaf( frame, tree->left->left );
            break;
    }

    if( strcmp( frame->name, main_method ) == 0 )
    {
        printf( "%d\n", program_value );
        exit(1);
    }

    return program_value;
}

void process_variables( ENVIRONMENT_FRAME *frame, NODE *tree )
{
    if ( tree == NULL ) return frame;
    if ( tree->left->type != LEAF ) return frame;
    if ( tree->right->type != '=' ) return frame;

    char *variable_name =  get_leaf( tree->right->left->left );
    int variable_value = 0;
    if ( tree->right->right->left->left == NULL )
    {
        variable_value  =  get_int_from_leaf( tree->right->right->left );
    }
    else
    {
        char* left_variable_name;
        char* right_variable_name;
        switch( tree->right->right->type )
        {
            case '+':
                left_variable_name = get_leaf( tree->right->right->left->left );
                right_variable_name = get_leaf( tree->right->right->right->left );
                variable_value = get_int_from_token( lookup_variable( frame->bindings, left_variable_name ) ) +
                                 get_int_from_token( lookup_variable( frame->bindings, right_variable_name ) );
                break;
            
            case '-':
                left_variable_name = get_leaf( tree->right->right->left->left );
                right_variable_name = get_leaf( tree->right->right->right->left );
                variable_value = get_int_from_token( lookup_variable( frame->bindings, left_variable_name ) ) -
                                 get_int_from_token( lookup_variable( frame->bindings, right_variable_name ) );
                break;
            
            case 42:
                left_variable_name = get_leaf( tree->right->right->left->left );
                right_variable_name = get_leaf( tree->right->right->right->left );
                variable_value = get_int_from_token( lookup_variable( frame->bindings, left_variable_name ) ) *
                                 get_int_from_token( lookup_variable( frame->bindings, right_variable_name ) );
                break;
            
            case '/':
                left_variable_name = get_leaf( tree->right->right->left->left );
                right_variable_name = get_leaf( tree->right->right->right->left );
                variable_value = get_int_from_token( lookup_variable( frame->bindings, left_variable_name ) ) /
                                 get_int_from_token( lookup_variable( frame->bindings, right_variable_name ) );
                break;

        }
    }

    TOKEN* value = new_token( CONSTANT );
    value->value = variable_value;

    ENVIRONMENT_BINDING *new_variable = define_variable_with_value( frame, previous_node, variable_name, value );
    previous_node = new_variable;
}

ENVIRONMENT_FRAME* process_parameters( ENVIRONMENT_FRAME *frame, NODE *parameters )
{
    if ( parameters == NULL ) return frame;

    if ( parameters->type == '~' )
    {
        char *param_name = get_leaf( parameters->right->left );

        TOKEN* value = (TOKEN*)malloc( sizeof( TOKEN ) );
        value = new_token( CONSTANT );
        value->value = 0;

        ENVIRONMENT_BINDING *new_variable = define_variable_with_value( frame, previous_node, param_name, value );
        previous_node = new_variable;
        return frame;
    }
    else
    {
        frame = process_parameters( frame, parameters->left );
        frame = process_parameters( frame, parameters->right );
        return frame;
    }
}

ENVIRONMENT_FRAME* process_function( ENVIRONMENT_FRAME *frame, NODE *return_type, NODE *function_parameters )
{
    char* return_type_as_char = get_leaf( return_type->left ); // should return 'int' or 'function'
    char* function_name = get_leaf( function_parameters->left->left ); // should return function name e.g. main.

    // Main method 'hack'
    if ( main_method == NULL )
      main_method = function_name;

    frame = update_environment_with_metadata( frame, function_name, return_type_as_char );

    // Function parameters
    if ( function_parameters->right != NULL )
    {
        frame = process_parameters( frame, function_parameters->right );
        frame = add_bindings_to_environment( frame, previous_node );
    }

    return frame;
}

int perform_eq_op_comparision( ENVIRONMENT_FRAME* frame, int one, int two, NODE* returnInformation )
{
    int returnValue = NULL;

    if( one == two )
    {
        returnValue = process_return( frame, returnInformation, NULL, NULL, NULL, NULL );
    }

    return returnValue;
}

ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME *frame, NODE *conditional, int operand )
{
    //printf( "Operand: %s\n", named( operand ) );

    TOKEN* left_var;
    TOKEN* right_var;
    char* left_fix; char* right_fix;
    char* left  = (char*)malloc(sizeof( char ) * 20);
    char* right = (char*)malloc(sizeof( char ) * 20);
    int returnValue;

    left_fix = get_leaf( conditional->left->left->left );
    right_fix = get_leaf( conditional->left->right->left );

    if ( strcmp( left_fix, "???" ) == 0 )
    {
        sprintf( left, "%d", get_int_from_leaf( conditional->left->left->left ) );
    }
    else
    {
        left = left_fix;
    }

    if ( strcmp( right_fix, "???" ) == 0 )
    {
        sprintf( right, "%d", get_int_from_leaf( conditional->left->right->left ) );
    }
    else
    {
        right = right_fix;
    }

    left_var = lookup_variable( frame->bindings, left );
    right_var = lookup_variable( frame->bindings, right );

    //printf( "left = %d... right = %d\n", left_var->value, right_var->value );

    switch( operand )
    {
        case EQ_OP:
            returnValue = perform_eq_op_comparision( frame, left_var->value, right_var->value, conditional->right );
            //printf( "return value %d\n", returnValue );
            break;

        case LE_OP:
            break;

        case GE_OP:
            break;

        case NE_OP:
            break;
    }

    frame->return_value = returnValue;

    return frame;
}
#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "C.tab.h"

/**
 * process_apply_params
 *
 * This function starts off with a blank linked list (valueList) and for each value found 
 * in the 'apply' part of the Abstract Syntax Tree, it puts it into the linked list. 
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   the relevant piece of abstract syntax tree we are looking at
 * @arg     RUNTIME_VALUES*         a linked list of pass-at-runtime values for "apply"
 * @returns RUNTIME_VALUES*         returns a linked list complete with variables
 */
RUNTIME_VALUES* process_apply_params( ENVIRONMENT_FRAME* frame, NODE* tree, RUNTIME_VALUES *valueList )
{
    // If there are no more elements in the tree, return the last value
    // which has references to all other values (a complete linked list)
    if ( tree == NULL ) return valueList;

    // We only care about values that are LEAFs, if they are not leafs then 
    // proceed to next iteration
    if ( tree->type != LEAF )
    {
        valueList = process_apply_params( frame, tree->left, valueList );
        valueList = process_apply_params( frame, tree->right, valueList );
    }
    else
    {
        // Given a leaf, convert it into an integer and return it to us
        int value = get_int_from_leaf( tree->left );
        
        // Create a new RUNTIME_VALUE node and set "next" to be the current valueList
        RUNTIME_VALUES *valuePtr = (RUNTIME_VALUES*)malloc( sizeof( RUNTIME_VALUES ) );
        valuePtr->next = valueList;
        valuePtr->value = value;
        return valuePtr;
    }
}

/**
 * process_apply
 *
 * Trigger function for "APPLY" statements in the Abstract Syntax Tree
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   function header decalaration respresented in AST (e.g. int main( a, b, c ))
 * @arg     NODE*                   function body represented in AST (e.g. { if( a == 1 ){ return true; } })
 * @arg     char*                   function name that the apply is being called from
 * @arg     NODE*                   function parameter values
 * @returns ENVIRONMENT_FRAME*      updated frame with frame_value set
 */
ENVIRONMENT_FRAME* process_apply( ENVIRONMENT_FRAME* frame, NODE *declaration, NODE *body, char *function_name, NODE *parameters )
{
    RUNTIME_VALUES *values = process_apply_params( frame, parameters, NULL );

    // Setup a new temporary environment which is discarded after this apply
    // The reason I do this is because "APPLY" receives values that are to be applied
    // to an existing function and are changeable. Should not alter existing function
    // environment, but add a temporary one with the new values.
    ENVIRONMENT_FRAME *tmpEnv = (ENVIRONMENT_FRAME *)setup_new_environment( NULL );
    tmpEnv->declaration = declaration;
    tmpEnv->body        = body;
    tmpEnv->name        = function_name; 
    tmpEnv->next        = frame->next;

    // Get our function variables (bindings), and set a reference to the firstBinding for later use
    ENVIRONMENT_BINDING *bindings = frame->bindings;
    ENVIRONMENT_BINDING *firstBinding = bindings;

    // Loop whilst we have values to apply to variables
    while( values != NULL )
    {
        TOKEN *newValue = new_token( CONSTANT );
        newValue->value = values->value;
        bindings->value = newValue;

        bindings = bindings->next;
        values = values->next;
    }

    // Set the variables for this temporary environment
    tmpEnv->bindings = firstBinding;

    // Now our temporary environment is fully set up we need to rerun the whole parser
    frame = parse_environment( tmpEnv, body );
    
    // Return completed frame
    return frame;
}

/**
 * process_leaf
 *
 * If the trigger is of type LEAF (e.g. return 10;) we just need to work out whether
 * it is a number or a variable, and return the appropriate integer value
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   leaf we are evaluating
 * @returns int                     evaluated leaf
 */
int process_leaf( ENVIRONMENT_FRAME *frame, NODE *leaf )
{
    int program_value;

    // This is a bit of a hackish way to work out of a char is a number
    // or a variable name. Idea discussed and came up with Matt Nicholls (mln24)
    // get_int_from_leaf will try casting it to an integer, if it cannot it will return
    // a constant MAX_INTEGER (which we felt would not be used in a program often)
    if( get_int_from_leaf( leaf ) != MAX_INTEGER )
    {
        // Leaf is a number, so we set program_value to it's value
        program_value = get_int_from_leaf( leaf );
    }
    else
    {
        // Leaf is infact a letter/word, lets see if it is a variable. if it is return its value
        // otherwise return MAX_INTEGER (get_int_from_token will return this when NaN)
        char* leaf_name = get_leaf( leaf );
        program_value = get_int_from_token( lookup_variable( frame->bindings, leaf_name ) );
    }

    return program_value;
}

/**
 * process_return
 *
 * Trigger for "RETURN" keyword in the Abstract Syntax Tree. Will evaluate the return, and return the results.
 * Return supports evaluating variables (e.g. return a;), evaluating numbers (return 100;), arithimetic of variables
 * (return x*2; return a+1;), and calling additional functions (return function(1,2,3);).
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   the relevant AST block (return and all its child leafs)
 * @arg     char*                   for "APPLY" scenarios we need to know what function we are already in
 * @arg     NODE*                   for "APPLY" scenarios we need to know the function decalaration (int main( int a, int b, int c ) )
 * @arg     NODE*                   for "APPLY" scenarios we need to know the function body itself (the code the function runs)
 * @arg     NODE*                   for "APPLY" scenarios we need to know the passed in parameters at runtime
 * @returns int                     evaluated return statement as an integer
 */
int process_return( ENVIRONMENT_FRAME *frame, NODE *tree, char *function_name, NODE *declaration, NODE *body, NODE *parameters )
{
    int left_int; int right_int;
    int program_value;

    // Are we running an apply function?
    if ( tree->left->type == APPLY || tree->left->left->type == APPLY )
    {
        NODE* treeCpy   = tree;
        right_int       = MAX_INTEGER;

        if ( tree->left->left->type == APPLY )
        {
            treeCpy     = tree->left;
            right_int   = get_value_from_tree( frame->bindings, treeCpy->right->left );
        }

        function_name   = get_leaf( treeCpy->left->left->left );
        declaration     = get_declaration_of_function( frame, function_name );
        body            = get_body_of_function( frame, function_name );
        parameters      = treeCpy->left->right;

        frame           = process_apply( frame, declaration, body, function_name, parameters );
        left_int        = frame->return_value;
    }
    else
    {
        if ( tree->left->left->left != NULL )
            left_int    = get_value_from_tree( frame->bindings, tree->left->left->left );
        else
            left_int    = get_value_from_tree( frame->bindings, tree->left->left );

        if ( tree->left->right != NULL )
            right_int   = get_value_from_tree( frame->bindings, tree->left->right->left );
    }

    // Work out what to do based on the trigger type (APPLY,IF,+,-,*,/)
    switch( tree->left->type )
    {
        case APPLY:
            program_value   = frame->return_value;
            break;

        case IF:
            frame           = process_conditional( frame, body, body->left->type );
            program_value   = frame->return_value;
            break;

        case '+':
            program_value   = left_int + ( ( right_int == MAX_INTEGER ) ? 0 : right_int );
            break;

        case '-':
            program_value   = left_int - ( ( right_int == MAX_INTEGER ) ? 0 : right_int );
            break;
          
        case 42:
            program_value   = left_int * ( ( right_int == MAX_INTEGER ) ? 1 : right_int );
            break;
          
        case '/':
            program_value   = left_int / ( ( right_int == MAX_INTEGER ) ? 1 : right_int );
            break;

        case LEAF:
            program_value   = process_leaf( frame, tree->left->left );
            break;
    }

    if ( tree->left->type == APPLY || tree->left->left->type == APPLY )
        frame           = frame->next;

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
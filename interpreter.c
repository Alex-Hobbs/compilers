#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "C.tab.h"

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
    ENVIRONMENT_FRAME *tmpEnv = (ENVIRONMENT_FRAME *) setup_new_environment( NULL );
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
 * process_conditional
 *
 * Trigger function called when a IF statement is found in the abstract syntax tree
 * This function gets the two values either side of the equality operand and evaluates
 * the expression, and if true returns the correct code to evaluate.
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   the conditional expression respresented in the AST
 * @arg     int                     the operand we are running ( ==, !=, <, >, <=, >= )
 * @returns ENVIRONMENT_FRAME*      frame updated with program value
 */
ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME *frame, NODE *conditional, int operand )
{
    TOKEN* left_value; TOKEN* right_value;
    char* left_leaf; char* right_leaf;

    // Allocate 100 bytes maximum to be used for a variable name
    // This limitation has to be put in place due to how sprintf
    // functions.
    char* left  = (char*) malloc( sizeof( char ) * 100 );
    char* right = (char*) malloc( sizeof( char ) * 100 );

    /**
     * Idea and code discussed between myself and Matt Nicholls (mln24) to 
     * try and find a way to convert a integer into a character
     *
     * This was done in order to not have to write new functions for
     * very small use cases.
     */
    NODE* values        = conditional->left;
    NODE* expression    = conditional->right;
    left_leaf           = get_leaf( values->left->left );
    right_leaf          = get_leaf( values->right->left );

    // If the left leaf was a pure number (get_leaf will return ??? as it is passed through named),
    // then convert the number into a character of itself. E.g. 1 becomes '1'
    if ( strcmp( left_leaf, "???" ) == 0 )
        sprintf( left, "%d", get_value_from_tree( frame->bindings, values->left->left ) );
    else // Otherwise use the default value (probably a variable name)
        left = left_leaf;

    if ( strcmp( right_leaf, "???" ) == 0 )
        sprintf( right, "%d", get_value_from_tree( frame->bindings, values->right->left ) );
    else
        right = right_leaf;

    // Lookup these two variables and get their values if they were not already numbers
    left_value   = lookup_variable( frame->bindings, left );
    right_value  = lookup_variable( frame->bindings, right );

    // Work out what to do based on the operand being run.
    // If statements support ==, <=, <, >=, >, !=
    int evaluation = 0;
    switch( operand )
    {
        case EQ_OP:
            if ( left_value->value == right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;

        case LE_OP:
            if ( left_value->value <= right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;

        // Less than but not equal to
        case L_OP:
            if( left_value->value < right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;

        case GE_OP:
            if ( left_value->value >= right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;

        // Greater than but not equal to
        case G_OP:
            if( left_value->value > right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;

        case NE_OP:
            if ( left_value->value != right_value->value )
                evaluation = process_return( frame, expression, NULL, NULL, NULL, NULL );
            break;
    }

    frame = set_environment_return_value( frame, evaluation );
    return frame;
}

ENVIRONMENT_FRAME* process_function( ENVIRONMENT_FRAME *frame, NODE *return_type, NODE *function_parameters )
{
    char* return_type_as_char = get_leaf( return_type->left ); // should return 'int' or 'function'
    char* function_name = get_leaf( function_parameters->left->left ); // should return function name e.g. main.

    // Main method 'hack', allows the system to know what the first runnable function in the program was
    if ( main_function == NULL ) main_function = function_name;

    frame = update_environment_with_metadata( frame, function_name, return_type_as_char );

    // Function parameters
    if ( function_parameters->right != NULL )
    {
        frame = process_parameters( frame, function_parameters->right );
        frame = add_bindings_to_environment( frame, previous_node );
    }

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
 * process_parameters
 *
 * Trigger function called when a function that has paramters is found in the AST
 * denoted by ~, similar to the process_variables function but sets up initial values
 * to be 0.
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   function parameters represented in the AST
 * @returns ENVIRONMENT_FRAME*      frame with parameters attached
 */
ENVIRONMENT_FRAME* process_parameters( ENVIRONMENT_FRAME *frame, NODE *parameters )
{
    if ( parameters == NULL ) return frame;

    if ( parameters->type == TILDA )
    {
        char *param_name = get_leaf( parameters->right->left );

        TOKEN* value = (TOKEN*)malloc( sizeof( TOKEN ) );
        value = new_token( CONSTANT );
        value->value = 0;

        ENVIRONMENT_BINDING *new_variable = define_variable_with_value( previous_node, param_name, value );
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
    else if ( tree->left->type != LEAF )
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

        case ADD:
            program_value   = left_int + ( ( right_int == MAX_INTEGER ) ? 0 : right_int );
            break;

        case SUBTRACT:
            program_value   = left_int - ( ( right_int == MAX_INTEGER ) ? 0 : right_int );
            break;
          
        case MULTIPLY:
            program_value   = left_int * ( ( right_int == MAX_INTEGER ) ? 1 : right_int );
            break;
          
        case DIVIDE:
            program_value   = left_int / ( ( right_int == MAX_INTEGER ) ? 1 : right_int );
            break;

        case MODULO:
            program_value   = left_int % ( ( right_int == MAX_INTEGER ) ? 1 : right_int );
            break;

        case LEAF:
            program_value   = process_leaf( frame, tree->left->left );
            break;
    }

    if ( tree->left->type == APPLY || tree->left->left->type == APPLY )
        frame           = frame->next;

    if( strcmp( frame->name, main_function ) == 0 )
    {
        printf( "%d\n", program_value );
        exit(1);
    }

    return program_value;
}

/**
 * process_variables
 *
 * Trigger function run when AST contains ~, and handles code such as int x = 1; in the function body
 *
 * @arg     ENVIRONMENT_FRAME*      current environment we are in
 * @arg     NODE*                   abstract syntax tree at the relevant node (~)
 * @returns void
 */
void process_variables( ENVIRONMENT_FRAME *frame, NODE *tree )
{
    if ( tree == NULL ) return frame;
    if ( tree->left->type != LEAF ) return frame;
    if ( tree->right->type != '=' ) return frame;

    int variable_value = 0;

    if ( is_leaf( tree->right->right->left ) == TRUE )
    {
        variable_value  =  get_value_from_tree( frame->bindings, tree->right->right->left );
    }
    else
    {
        NODE *variable_values = tree->right->right;

        // Switch based on the operation we are looking at
        switch( variable_values->type )
        {
            case ADD:
                variable_value = get_value_from_tree( frame->bindings, get_leaf( variable_values->left->left ) ) +
                                 get_value_from_tree( frame->bindings, get_leaf( variable_values->right->left ) );
                break;
            
            case SUBTRACT:
                variable_value = get_value_from_tree( frame->bindings, variable_values->left->left ) -
                                 get_value_from_tree( frame->bindings, variable_values->right->left );
                break;
            
            case MULTIPLY:
                variable_value = get_value_from_tree( frame->bindings, variable_values->left->left ) *
                                 get_value_from_tree( frame->bindings, variable_values->right->left );
                break;
            
            case DIVIDE:
                variable_value = get_value_from_tree( frame->bindings, variable_values->left->left ) /
                                 get_value_from_tree( frame->bindings, variable_values->right->left );
                break;

            case MODULO:
                variable_value = get_value_from_tree( frame->bindings, variable_values->left->left ) %
                                 get_value_from_tree( frame->bindings, tree->right->right->right->left );
                break;
        }
    }


    char *variable_name = get_leaf( tree->right->left->left );
    TOKEN* value        = new_token( CONSTANT );
    value->value        = variable_value;

    ENVIRONMENT_BINDING *new_variable = define_variable_with_value( previous_node, variable_name, value );
    previous_node                     = new_variable;
}
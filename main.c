#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "environment.h"

int process_return( ENVIRONMENT_FRAME*, NODE* );
ENVIRONMENT_BINDING *previous_node = NULL;
char* main_method = NULL;

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

char *get_leaf(NODE *tree)
{
    TOKEN *t = (TOKEN *)tree;
    if (t->type == CONSTANT) return named( t->value );
    else if (t) return t->lexeme;
}

int get_int_from_leaf(NODE *tree)
{
    TOKEN *t = (TOKEN *)tree;
    if (t->type == CONSTANT) return t->value;
    else if (t) return 0;
}

int get_int_from_token(TOKEN *tree)
{
    if ( tree == NULL ) return -1;
    if (tree->type == CONSTANT) return tree->value;
    else if (tree) return tree->lexeme;
    else return 0;
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar('  ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{
    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar('  ');
      printf("%s\n", named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

ENVIRONMENT_FRAME* setup_new_environment( ENVIRONMENT_FRAME *neighbour )
{
    ENVIRONMENT_FRAME *base = (ENVIRONMENT_FRAME*)malloc( sizeof( ENVIRONMENT_FRAME ) );
    base->next = neighbour;
    return base;
}

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

ENVIRONMENT_FRAME* process_apply( ENVIRONMENT_FRAME* frame, NODE *tree )
{
    char *function_name = get_leaf( tree->left->left );

    NODE *declaration   = get_declaration_of_function( frame, function_name );
    NODE *body          = get_body_of_function( frame, function_name );

    // Start of the search/replace at beginning of function variables
    NODE *parameters    = tree->right;
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
    int returnValue = process_return( tmpEnv, body );
    frame->return_value = returnValue;

    return frame;
}

int process_return( ENVIRONMENT_FRAME *frame, NODE *tree )
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
            frame = process_apply( frame, tree->left );
            program_value = frame->return_value;
            frame = frame->next;
            break;

        case '+':
          if( tree->left->left->type == APPLY )
          {
                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, tree->left->left );

                if ( right_int != 0 )
                {
                    printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
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
                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, tree->left->left );

                if ( right_int != 0 )
                {
                    printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
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
                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, tree->left->left );

                if ( right_int != 0 )
                {
                    printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
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
                right_int = get_int_from_leaf( tree->left->right->left );
                frame = process_apply( frame, tree->left->left );

                if ( right_int != 0 )
                {
                    printf( "right integer = %d, left integer = %d\n", right_int, frame->return_value );
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
            left_variable_name = get_leaf( tree->left->left );
            program_value = get_int_from_token( lookup_variable( frame->bindings, left_variable_name ) );
            break;
    }

    //printf( "frame name = %s, main = %s\n", frame->name, main_method );

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

ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME *frame, NODE *conditional, int operand )
{
    printf( "Operand: %s\n", named( operand ) );

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

    printf( "left = %d... right = %d\n", left_var->value, right_var->value );

    switch( operand )
    {
        case EQ_OP:
        printf("eqop\n");
            if ( (int) left_var->value != (int) right_var->value )
            {
                returnValue = process_return( frame, conditional->right );
                printf( "returned %d\n", returnValue );
            }
            break;
        
        case LE_OP:
            break;

        case GE_OP:
            break;

        case NE_OP:
            break;
    }

    return frame;
}

ENVIRONMENT_FRAME* parse_environment( ENVIRONMENT_FRAME *current_frame, NODE *tree )
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

        switch( tree->type )
        {
            // Entered a new function
            case 'D':
                new_frame = extend_environment( current_frame, NULL );
                new_frame = store_function( new_frame, tree->left, tree->right );

                previous_node = NULL;
                current_frame = new_frame;
                break;
            
            case RETURN:
                process_return( current_frame, tree );
                //printf( "Current Frame = %s\n", current_frame->name );
                break;

            case 'd':
                current_frame = process_function( current_frame, tree->left, tree->right );
                //printf( "Current Frame = %s\n", current_frame->name );
                break;

            // Found a list of variables
            case '~':
                process_variables( current_frame, tree );
                current_frame = add_bindings_to_environment( current_frame, previous_node );
                //printf( "Current Frame = %s\n", current_frame->name );
                break;

            case IF:
                current_frame = process_conditional( current_frame, tree, tree->left->type );
                break;

            //default:
              //printf( "Found nothing, looked for %c\n", tree->type );
        }
    }

    current_frame = parse_environment( current_frame, tree->left );
    current_frame = parse_environment( current_frame, tree->right );
    return current_frame;
}

extern int yydebug;
extern NODE* yyparse(void);
extern NODE* ans;
extern void init_symbtable(void);

int main(int argc, char** argv)
{
    NODE* tree;
    if (argc>1 && strcmp(argv[1],"-d")==0) yydebug = 1;
    init_symbtable();
    //printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    //printf("parse finished with %p\n", tree);
    print_tree(tree);

    ENVIRONMENT_FRAME *base = setup_new_environment( NULL );
    base = parse_environment(base, tree);

    return 0;
}

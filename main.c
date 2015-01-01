#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "environment.h"

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
    if (tree->type == CONSTANT) return tree->value;
    else if (tree) return 0;
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i=0; i<level; i++) putchar(' ');
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
      for(i=0; i<level; i++) putchar(' ');
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

ENVIRONMENT_FRAME* process_variables( ENVIRONMENT_FRAME *frame, NODE *tree )
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
                printf( "%s + %s\n", left_variable_name, right_variable_name );
                break;
        }
    }

    ENVIRONMENT_BINDING *new_variable = define_variable_with_value( frame, NULL, variable_name, variable_value );
    //previous_node = new_variable;
    frame = add_bindings_to_environment( frame, new_variable );
    return frame;
}

ENVIRONMENT_FRAME* process_function( ENVIRONMENT_FRAME *frame, NODE *return_type, NODE *function_parameters )
{
    char* return_type_as_char = get_leaf( return_type->left ); // should return 'int' or 'function'
    char* function_name = get_leaf( function_parameters->left->left ); // should return function name e.g. main.

        frame = update_environment_with_metadata( frame, function_name, return_type_as_char );

    // Function parameters
    if ( function_parameters->right != NULL )
    {

    }

    return frame;
}

ENVIRONMENT_FRAME* parse_environment( ENVIRONMENT_FRAME *current_frame, NODE *tree )
{
    if (tree==NULL) return current_frame;

    if (tree->type == LEAF)
    {
        printf( "Found leaf:\n" );
        print_leaf( tree, 1 );
        printf( "\n" );
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
                new_frame = parse_environment( new_frame, tree->left );
                new_frame = parse_environment( new_frame, tree->right );
                return new_frame;

            case 'd':
                return process_function( current_frame, tree->left, tree->right );

            // Found a list of variables
            case '~':
                return process_variables( current_frame, tree );
            
            case RETURN:
                //return process_return()

            default:
              printf( "Found nothing, looked for %c\n", tree->type );
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
    printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);

    ENVIRONMENT_FRAME *base = setup_new_environment( NULL );
    base = parse_environment(base, tree);

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include "C.tab.h"
#include "common.h"

int get_value_from_tree( ENVIRONMENT_BINDING *binding, NODE *value )
{
    int test_value;
  
    test_value = get_int_from_leaf( value );

    // If the value we get back is MAX_INTEGER, then we are not a number
    // therefore lookup variable value.
    if ( test_value == MAX_INTEGER )
    {
        test_value = get_int_from_token( lookup_variable( binding, get_leaf( value ) ) );

        // We're still a letter? We cannot apply arithmetic to a number, error out.
        if ( test_value == MAX_INTEGER )
        {
            return NULL;
        }
    }

    return (int) test_value;
}

bool is_leaf( NODE *leaf )
{
    if ( leaf->left == NULL )
        return true;
      
    return false;
}

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
    return get_int_from_token(t);
}

int get_int_from_token(TOKEN *tree)
{
    if ( tree == NULL || tree->value == NULL ) return MAX_INTEGER;
    if (tree->type == CONSTANT) return tree->value;
    else if (tree) return (int) atoi( tree->lexeme );
    else return MAX_INTEGER;
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


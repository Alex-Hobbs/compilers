#ifndef __COMMON_H
#define __COMMON_H
#include "token.h"
#include "nodes.h"

#define MAX_INTEGER 2147483646 // max integer - 1 for a 32-bit system
#define TRUE 1
#define FALSE 0
#define L_OP 60
#define G_OP 62
#define MULTIPLY 42
#define ADD 43
#define SUBTRACT 45
#define DIVIDE 47
#define MODULO 37
#define ENVIRONMENT 68
#define DECLARATION 100
#define TILDA 126

char*				main_function;

int is_leaf(NODE*);
char* get_leaf(NODE*);
char* named(int);
int get_int_from_leaf(NODE*);
int get_int_from_token(TOKEN*);
void print_leaf(NODE*, int);
void print_tree(NODE*);

#endif

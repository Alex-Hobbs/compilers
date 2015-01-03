#ifndef __COMMON_H
#define __COMMON_H
#include "token.h"
#include "nodes.h"
#define MAX_INTEGER 2147483646 // max integer - 1 for a 32-bit system

char* get_leaf(NODE*);
char* named(int);
int get_int_from_leaf(NODE*);
int get_int_from_token(TOKEN*);
void print_leaf(NODE*, int);
void print_tree(NODE*);

#endif

#ifndef __COMMON_H
#define __COMMON_H
#include "token.h"
#include "nodes.h"

char* get_leaf(NODE*);
char* named(int);
int get_int_from_leaf(NODE*);
int get_int_from_token(TOKEN*);
void print_leaf(NODE*, int);
void print_tree(NODE*);

#endif


#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"
#include "token.h"
#include "common.h"
#include "environment.h"

char* main_method;
NODE *previous_node;
ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME*, NODE*, int );
int process_return( ENVIRONMENT_FRAME*, NODE*, char*, NODE*, NODE*, NODE* );

#endif
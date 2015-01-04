
#ifndef __INTERPRETER_H
#define __INTERPRETER_H
#include "nodes.h"
#include "token.h"
#include "common.h"
#include "environment.h"

NODE*				previous_node;

ENVIRONMENT_FRAME* 	process_apply( ENVIRONMENT_FRAME*, NODE*, NODE*, char*, NODE* );
RUNTIME_VALUES* 	process_apply_params( ENVIRONMENT_FRAME*, NODE*, RUNTIME_VALUES* );
ENVIRONMENT_FRAME* 	process_conditional( ENVIRONMENT_FRAME*, NODE*, int );
ENVIRONMENT_FRAME* 	process_for( ENVIRONMENT_FRAME*, NODE* );
ENVIRONMENT_FRAME* 	process_function( ENVIRONMENT_FRAME*, NODE*, NODE* );
int 				process_leaf( ENVIRONMENT_FRAME*, NODE* );
ENVIRONMENT_FRAME* 	process_parameters( ENVIRONMENT_FRAME*, NODE* );
int 				process_return( ENVIRONMENT_FRAME*, NODE*, char*, NODE*, NODE*, NODE* );
void 				process_variables( ENVIRONMENT_FRAME*, NODE* );
ENVIRONMENT_FRAME* 	process_while( ENVIRONMENT_FRAME*, NODE* );

#endif
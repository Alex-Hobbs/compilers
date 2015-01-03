
#ifndef __NODES_H
#define __NODES_H
#include "nodes.h"
#include "token.h"
#include "environment.h"

ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME*, NODE*, int );
int process_return( ENVIRONMENT_FRAME*, NODE*, char*, NODE*, NODE*, NODE* );

#endif
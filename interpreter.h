
#ifndef __NODES_H
#define __NODES_H
#include "token.h"

ENVIRONMENT_FRAME* process_conditional( ENVIRONMENT_FRAME*, NODE*, int );
int process_return( ENVIRONMENT_FRAME*, NODE*, char*, NODE*, NODE*, NODE* );

#endif
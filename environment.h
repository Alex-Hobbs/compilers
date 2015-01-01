#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H
#include "token.h"

typedef struct env_binding
{
  char					   *name;
  TOKEN					   *value;
  struct env_binding	   *next;
} ENVIRONMENT_BINDING;

typedef struct env_frame
{
	ENVIRONMENT_BINDING			*bindings;
	struct env_frame			*next;
} ENVIRONMENT_FRAME;

TOKEN* get_value_from_variable(ENVIRONMENT_BINDING*, char*);
ENVIRONMENT_FRAME* extend_environment( ENVIRONMENT_FRAME*, ENVIRONMENT_BINDING* );

#endif

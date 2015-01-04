#ifndef __TOKEN_H
#define __TOKEN_H
#define TEST_MODE 0

typedef struct TOKEN
{
  int           type;
  char          *lexeme;
  int           value;
  struct TOKEN  *next;
} TOKEN;

extern TOKEN* new_token(int);

#endif

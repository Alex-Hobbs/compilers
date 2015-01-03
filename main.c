#include <stdio.h>
#include <ctype.h>
#include "common.h"
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "environment.h"
#include "interpreter.h"

extern int yydebug;
extern NODE* yyparse(void);
extern NODE* ans;
extern void init_symbtable(void);

int main(int argc, char** argv)
{
    NODE* tree;
    if (argc>1 && strcmp(argv[1],"-d")==0) yydebug = 1;
    init_symbtable();
    //printf("--C COMPILER\n");
    yyparse();
    tree = ans;
    //printf("parse finished with %p\n", tree);
    //print_tree(tree);

    ENVIRONMENT_FRAME *base = setup_new_environment( NULL );
    base = parse_environment(base, tree);

    return 0;
}

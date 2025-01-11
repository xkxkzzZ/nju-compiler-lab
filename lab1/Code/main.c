#include <stdio.h>
#include <stdlib.h>

#define mDEBUG 0


typedef struct Node {
    char* name;
    int ID;
    struct Node* parent;
    struct Node* child;
    struct Node* sibling;
    char* moreinfo;
} Node;
#include "syntax.tab.h"

extern int yyparse();
extern int yyrestart(FILE *f);
extern int errorflag;
extern int lexerror[404];

extern Node* root;
extern void printTree(Node* node, int depth);
extern void findTreeError(Node* node);
extern void printLexerror();
extern void printParerror();



int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror(argv[1]);
            return 1;
        }
        yyrestart(f);
    }
    int flag = 1;
    while(yyparse() != 0) flag = 0;
    if (errorflag) {
        // printf("error\n");
        findTreeError(root);
        printLexerror();
        printParerror();
    }

    else {
        // printf("success\n");
        if(!mDEBUG) printTree(root, 0);
    }
    if(mDEBUG)   printTree(root, 0);
}

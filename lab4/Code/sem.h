#include "qwq.h"
#define HASHSIZE 0x3fff

typedef struct Operand_ Operand;

struct Type_ {
    enum { INT, FLOAT, ARRAY, STRUCTURE, STRUCTVAR, FUNC, WRONGFUNC } kind;
    union {
        int basic;
        struct
        {
            Type* elem;
            int size;
        } array;
        FieldList* structure;
        FieldList* structvar;
        struct
        {
            Type* ret;
            int argc;
            FieldList* args;
        } function;
    } u;
};

struct FieldList_ {
    char* name; 
    Type* type;
    FieldList* tail;
};

struct TableList_ {
    char* name;
    Type* type;
    unsigned size;
    Operand* op;
    TableList* next;
};


void Program(Node *node);
void ExtDefList(Node *node);
void ExtDef(Node *node);
void Specifier(Node *node);
void ExtDecList(Node *node);
void FunDec(Node *node);
void CompSt(Node *node);
void VarDec(Node *node);
void StructSpecifier(Node *node);
void OptTag(Node *node);
void DefList(Node *node);
void Tag(Node *node);
void VarList(Node *node);
void ParamDec(Node *node);
void StmtList(Node *node);
void Stmt(Node *node);
void Exp(Node *node);
void Def(Node *node);
void DecList(Node *node);
void Dec(Node *node);
void Args(Node *node);



unsigned hash_pjw(char *name);
void initTable();
TableList* search(char* name);
void insert(TableList* item);

int structcmp(FieldList* s1, FieldList* s2);
int arraycmp(Type* t1, Type* t2);
int funccmp(FieldList* arg1, FieldList* arg2);
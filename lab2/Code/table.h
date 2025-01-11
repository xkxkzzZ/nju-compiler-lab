
#ifndef TABLE_H
#define TABLE_H

#define HASHSIZE 0x3fff

typedef struct Operand_ Operand;
typedef struct Type_ Type;
typedef struct FieldList_ FieldList;
typedef struct TableList_ TableList;
extern TableList* hashTable[HASHSIZE + 1];

struct Type_ {
    enum { eINT,
        eFLOAT,
        ARRAY,
        STRUCTURE,
        STRUCTVAR,
        FUNC,
        WRONGFUNC } kind;
    union {
        int basic;
        struct
        {
            Type* elem;
        } array;
        FieldList* structure;
        FieldList* structvar;
        struct
        {
            Type* ret;
            int argc;
            FieldList* args;
        } func;
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
    // unsigned size;
    Operand* op;
    TableList* next;
};

unsigned hash(char* name);
void initTable();
TableList* search(char* name);
void insert(TableList* item);
void printType(Type* type, int depth);
void printTable();

FieldList* newFieldList(char* name, Type* type, FieldList* tail);
TableList* newTableList(char* name, Type* type, Operand* op, TableList* next);

#endif
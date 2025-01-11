#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"

typedef unsigned char uint8_t;

typedef enum {
    Int,
    Float,
    Id,
    TYpe,
    Relop,
    Ter,
    Nter,
    Null
} MyType;

typedef struct Node_ Node;

struct Node_ {
    char* name;
    MyType type;
    int line;
    int no;
    union {
        unsigned type_int;
        float type_float;
        char type_str[40];
    } val;
    FieldList* syn;
    FieldList* inh;
    Node* parent;
    Node* child;
    Node* next;
    uint8_t instruct;
};

Node* newNode(char* _name, MyType _type, char* text);
Node* newnewNode(char* _name, int _no, MyType _type, int line);
void addchild(Node* parent, Node* child);
void outPut(Node* node, int dep);
Node* get_child(Node* node, int index);

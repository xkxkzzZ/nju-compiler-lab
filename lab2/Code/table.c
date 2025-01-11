#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "table.h"

unsigned hash(char* name)
{
    unsigned val = 0;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        val = val % HASHSIZE;
    }
    return val;
}

void initTable()
{
    TableList* item;
    FieldList* arg;
    Type* type;
    Type* func;
    for (int i = 0; i < HASHSIZE + 1; ++i) {
        hashTable[i] = NULL;
    }
    type = (Type*)malloc(sizeof(Type));
    type->kind = eINT;
    func = (Type*)malloc(sizeof(Type));
    func->kind = FUNC;
    func->u.func.argc = 0;
    func->u.func.args = NULL;
    func->u.func.ret = type;
    item = newTableList("read", func, NULL, NULL);
    insert(item);
    arg = newFieldList("", type, NULL);
    func = (Type*)malloc(sizeof(Type));
    func->kind = FUNC;
    func->u.func.argc = 1;
    func->u.func.args = arg;
    func->u.func.ret = type;

    item = newTableList("write", func, NULL, NULL);
    insert(item);
}

TableList* search(char* name)
{
    unsigned index = hash(name);
    for (TableList* node = hashTable[index]; node; node = node->next) {
        if (!strcmp(name, node->name))
            return node;
    }
    return NULL;
}

void insert(TableList* item)
{
    unsigned index = hash(item->name);
    item->next = hashTable[index];
    hashTable[index] = item;
}

FieldList* newFieldList(char* name, Type* type, FieldList* tail)
{
    FieldList* fl = (FieldList*)malloc(sizeof(FieldList));
    fl->name = name;
    fl->type = type;
    fl->tail = tail;
    return fl;
}

TableList* newTableList(char* name, Type* type, Operand* op, TableList* next)
{
    TableList* tl = (TableList*)malloc(sizeof(TableList));
    tl->name = name;
    tl->type = type;
    tl->op = op;
    tl->next = next;
    return tl;
}


void printType(Type* type, int depth)
{
    // assert(type);
    for (int i = 0; i < depth; ++i)
        printf("  ");
    printf("type: ");
    switch (type->kind) {
    case eINT:
        printf("int\n");
        break;
    case eFLOAT:
        printf("float\n");
        break;
    case ARRAY:
        // printf("array, size: %d\n", type->u.array.size);
        printf("array\n");
        printType(type->u.array.elem, depth + 1);
        break;
    case STRUCTURE: {
        printf("structure\n");
        FieldList* p = type->u.structure;
        while (p) {
            for (int i = 0; i < depth + 1; ++i)
                printf("  ");
            printf("name: %s\n", p->name);
            printType(p->type, depth + 1);
            p = p->tail;
        }
        break;
    }
    case STRUCTVAR: {
        printf("structvar\n");
        FieldList* p = type->u.structvar;
        while (p) {
            for (int i = 0; i < depth + 1; ++i)
                printf("  ");
            printf("name: %s\n", p->name);
            printType(p->type, depth + 1);
            p = p->tail;
        }
    } break;
    case FUNC: {
        printf("function\n");
        Type* ret = type->u.func.ret;
        for (int i = 0; i < depth + 1; ++i)
            printf("  ");
        printf("return:\n");
        printType(ret, depth + 2);
        for (int i = 0; i < depth + 1; ++i)
            printf("  ");
        printf("argc: %d\n", type->u.func.argc);
        for (int i = 0; i < depth + 1; ++i)
            printf("  ");
        printf("args:\n");
        FieldList* p = type->u.func.args;
        while (p) {
            for (int i = 0; i < depth + 2; ++i)
                printf("  ");
            printf("name: %s\n", p->name);
            printType(p->type, depth + 2);
            p = p->tail;
        }
    } break;
    default:
        // printf("%d\n", type->kind);
        break;
    }
}

void printTable()
{
    printf("---------------------------\n");
    for (int i = 0; i < HASHSIZE + 1; ++i) {
        if (!hashTable[i])
            continue;
        else {
            TableList* p = hashTable[i];
            // if (p->next) {
            //     ++collision;
            // }
            while (p) {
                if (p->type->kind != WRONGFUNC) {
                    printf("name: %s\n", p->name);
                    printType(p->type, 0);
                    printf("---------------------------\n");
                }
                p = p->next;
            }
        }
    }
    // printf("collision: %d\n", collision);
}
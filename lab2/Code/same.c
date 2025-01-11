#include "same.h"
#include <string.h>

int same_struct(FieldList* s1, FieldList* s2)
{
    if (strcmp(s1->name, s2->name) == 0) {
        return 1;
    }
    return 0;
}

int same_array(Type* t1, Type* t2)
{
    int res;
    while (t1 && t2) {
        if (t1 == t2)
            return 1;
        else if (t1->u.array.elem->kind != t2->u.array.elem->kind) {
            return 0;
        } else if (t1->u.array.elem->kind == STRUCTURE || t1->u.array.elem->kind == STRUCTVAR) {
            res = same_struct(t1->u.array.elem->u.structure, t2->u.array.elem->u.structure);
            if (!res)
                return 0;
            else
                return 1;
        } else if (t1->u.array.elem->kind != ARRAY) {
            return 1;
        }
        t1 = t1->u.array.elem;
        t2 = t2->u.array.elem;
    }
    if (t1 || t2)
        res = 0;
    return res;
}

int same_func(FieldList* arg1, FieldList* arg2)
{
    int res;
    while (arg1 && arg2) {
        if (arg1 == arg2)
            return 1;
        else if (arg1->type->kind != arg2->type->kind) {
            return 0;
        } else if (arg1->type->kind == STRUCTVAR) {
            res = same_struct(arg1->type->u.structvar, arg2->type->u.structvar);
            if (!res)
                return 0;
        } else if (arg1->type->kind == ARRAY) {
            res = same_array(arg1->type, arg2->type);
            if (!res)
                return 0;
        } else
            res = 1;
        arg1 = arg1->tail;
        arg2 = arg2->tail;
    }
    if (arg1 || arg2)
        res = 0;
    return res;
}
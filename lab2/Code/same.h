#include "table.h"

#ifndef SAME_H
#define SAME_H

int same_struct(FieldList* s1, FieldList* s2);
int same_array(Type* t1, Type* t2);
int same_func(FieldList* arg1, FieldList* arg2);

#endif
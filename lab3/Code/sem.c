#include "sem.h"

char msg[1024];
int collision = 0;
TableList* hashTable[HASHSIZE + 1];



FieldList* newFieldList(char* name, Type* type, FieldList* tail)
{
    FieldList* fl = (FieldList*)malloc(sizeof(FieldList));
    fl->name = name;
    fl->type = type;
    fl->tail = tail;
    return fl;
}

TableList* newTableList(char* name, Type* type, unsigned size,Operand* op, TableList* next)
{
    TableList* tl = (TableList*)malloc(sizeof(TableList));
    tl->name = name;
    tl->type = type;
    tl->size = size;
    tl->op = op;
    tl->next = next;
    return tl;
}


Type* newEmptytype()
{
    Type* type = (Type*)malloc(sizeof(Type));
    return type;
}

Type* typewithKind(int kind)
{
    Type* type = newEmptytype();
    type->kind = kind;
    return type;
}

void Program(Node* node)
{
    if (!strcmp("ExtDefList", node->child->name)) {
        ExtDefList(node->child);
    }
}

void ExtDefList(Node* node)
{
    if (node->no == 2) // ExtDef ExtDefList
    {
        ExtDef(node->child);
        ExtDefList(getChild(node, 1));
    }
}

void ExtDef(Node* node)
{
    Type* type = newEmptytype();
    FieldList* attr = newFieldList(NULL, type, NULL);
    switch (node->no) {
    case 1: // Specifier ExtDecList SEMI
        node->child->inh = attr;
        Specifier(node->child);
        getChild(node, 1)->inh = node->child->syn;
        ExtDecList(getChild(node, 1));
        break;
    case 2: // Specifier SEMI
        node->child->inh = attr;
        Specifier(node->child);
        break;
    case 3: // Specifier FunDec CompSt
        type->kind = FUNC;
        node->child->inh = attr;
        getChild(node, 1)->inh = attr;
        Specifier(node->child);
        if (node->child->syn->type->kind == STRUCTURE) {
            Type* type = typewithKind(STRUCTVAR);
            type->u.structvar = node->child->syn->type->u.structure;
            node->child->inh->type->u.function.ret = type;

        } else {
            node->child->inh->type->u.function.ret = node->child->syn->type;
        }
        node->child->inh->type->u.function.argc = 0;
        node->child->inh->type->u.function.args = NULL;
        FunDec(getChild(node, 1));
        getChild(node, 2)->inh = node->child->syn;
        CompSt(getChild(node, 2));
        break;
    default:
        break;
    }
}

void Specifier(Node* node)
{
    Type* type = newEmptytype();
    switch (node->no) {
    case 1: // TYPE
        if (!strcmp("int", node->child->val.type_str)) {
            type->kind = INT;
        } else {
            type->kind = FLOAT;
        }
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 2: // StructSpecifier
    {
        type->kind = STRUCTURE;
        type->u.structure = NULL;
        node->child->inh = newFieldList(NULL, type, NULL);
        StructSpecifier(node->child);
        node->syn = node->child->syn;
    } break;
    default:
        break;
    }
}

void ExtDecList(Node* node)
{
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec COMMA ExtDecList
    {
        getChild(node, 2)->inh = node->inh;
        ExtDecList(getChild(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = getChild(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void FunDec(Node* node)
{
    insert(newTableList(node->child->val.type_str, node->inh->type, 0, NULL, NULL));
    switch (node->no) {
    case 1: // ID LP VarList RP
        getChild(node, 2)->inh = node->inh;
        VarList(getChild(node, 2));
        node->inh->type->u.function.args = getChild(node, 2)->syn;
        break;
    case 2: // ID LP RP
        node->inh->type->u.function.args = NULL;
        break;
    default:
        break;
    }
}

void CompSt(Node* node)
{
    getChild(node, 1)->inh = node->inh;
    getChild(node, 2)->inh = node->inh;
    DefList(getChild(node, 1));
    StmtList(getChild(node, 2));
}


void OptTag(Node* node)
{
    if (node->no == 2) // ID
    {
        node->inh->name = node->child->val.type_str;
        insert(newTableList(node->child->val.type_str, node->inh->type, 0, NULL, NULL));

    }
}

void VarDec(Node* node)
{
    TableList* res;
    switch (node->no) {
    case 1: // ID
        res = search(node->child->val.type_str);
        if (!strcmp(node->child->val.type_str, "id_jAh9_Lg")) {
            int a = 0;
        }
        TableList* item = newTableList(node->child->val.type_str, node->inh->type, 0, NULL, NULL);
        // FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        switch (node->inh->type->kind) {
        case INT:
            item->type = node->inh->type;
            node->syn = newFieldList(node->child->val.type_str, node->inh->type, NULL);
            item->size = 4;
            break;
        case FLOAT:
            item->type = node->inh->type;
            node->syn = newFieldList(node->child->val.type_str, node->inh->type, NULL);
            item->size = 4;
            break;
        case ARRAY:
            item->type = node->inh->type;
            node->syn = newFieldList(node->child->val.type_str, node->inh->type, NULL);
            break;
        case STRUCTVAR:
        case STRUCTURE: {
            Type* type = (Type*)malloc(sizeof(Type));
            type->kind = STRUCTVAR;
            type->u.structvar = node->inh->type->u.structure;
            item->type = type;
            node->syn = newFieldList(node->child->val.type_str, type, NULL);
        } break;
        default:
            break;
        }
        insert(item);
        break;
    case 2: // VarDec LB INT RB
    {
        if (node->instruct)
            node->child->instruct = 1;
        Type* type = typewithKind(ARRAY);
        type->u.array.size = getChild(node, 2)->val.type_int;
        if (node->inh->type->kind == STRUCTURE) {
            Type* p = typewithKind(STRUCTVAR);
            p->u.structvar = node->inh->type->u.structure;
            type->u.array.elem = p;
        } else {
            type->u.array.elem = node->inh->type;
        }
        node->child->inh = newFieldList(NULL, type, NULL);
        VarDec(node->child);
        node->syn = node->child->syn;
        break;
    }
    default:
        break;
    }
}

void StructSpecifier(Node* node)
{
    switch (node->no) {
    case 1: // STRUCT OptTag LC DefList RC
    {
        Type* type = typewithKind(STRUCTURE);
        FieldList* field = newFieldList(NULL, type, NULL);
        getChild(node, 1)->inh = node->inh;
        getChild(node, 3)->inh = node->inh;
        getChild(node, 3)->instruct = 1;
        OptTag(getChild(node, 1));
        DefList(getChild(node, 3));
        if (!node->inh->type->u.structure) {
            node->inh->type->u.structure = getChild(node, 3)->syn;
        } else {
            FieldList* p = node->inh->type->u.structure;
            if (p->type->kind == STRUCTURE) {
                p->type->u.structure = field;
            } else {
                p->tail = field;
            }
        }
        node->syn = node->inh;
    } break;
    case 2: // STRUCT Tag
        node->inh->type->u.structure = NULL;
        getChild(node, 1)->inh = node->inh;
        Tag(getChild(node, 1));
        node->syn = node->inh;
        break;
    default:
        break;
    }
}

void DefList(Node* node)
{
    if (node->no == 2) // Def DefList
    {
        if (node->instruct) {
            node->child->instruct = 1;
            getChild(node, 1)->instruct = 1;
        }
        node->child->inh = node->inh;
        getChild(node, 1)->inh = node->inh;
        Def(node->child);
        node->syn = node->child->syn;
        DefList(getChild(node, 1));
        FieldList* p = node->syn;
        if (!p)
            return;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = getChild(node, 1)->syn;
    }
}

void VarList(Node* node)
{
    ParamDec(node->child);
    if (node->child->syn)
        node->inh->type->u.function.argc++;
    else {
        node->inh->type->kind = WRONGFUNC;
        return;
    }
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // ParamDec COMMA VarList
    {
        getChild(node, 2)->inh = node->inh;
        VarList(getChild(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = getChild(node, 2)->syn;
    } break;
    case 2: // ParamDec
        break;
    default:
        break;
    }
}
void Tag(Node* node)
{
    TableList* res = search(node->child->val.type_str);
    if (!res) {
        node->inh->name = node->child->val.type_str;
    } else {
        node->inh->name = res->name;
        node->inh->type = res->type;
    }
}


void ParamDec(Node* node)
{
    Specifier(node->child);
    getChild(node, 1)->inh = node->child->syn;
    VarDec(getChild(node, 1));
    node->syn = getChild(node, 1)->syn;
    if (getChild(node, 1)->syn) {
        node->syn = newFieldList(getChild(node, 1)->syn->name, getChild(node, 1)->syn->type, NULL);
    } else {
        node->syn = NULL;
    }
}

void StmtList(Node* node)
{
    if (node->no == 2) {
        node->child->inh = node->inh;
        getChild(node, 1)->inh = node->inh;
        Stmt(node->child);
        StmtList(getChild(node, 1));
    }
}

void Stmt(Node* node)
{
    switch (node->no) {
    case 1: // Exp SEMI
        Exp(node->child);
        break;
    case 2: // CompSt
        node->child->inh = node->inh;
        CompSt(node->child);
        break;
    case 3: // RETURN Exp SEMI
        Exp(getChild(node, 1));
        break;
    case 4: // IF LP Exp RP Stmt
        getChild(node, 4)->inh = node->inh;
        Exp(getChild(node, 2));
        Stmt(getChild(node, 4));
        break;
    case 5: // IF LP Exp RP Stmt ELSE Stmt
        getChild(node, 4)->inh = node->inh;
        getChild(node, 6)->inh = node->inh;
        Exp(getChild(node, 2));
        Stmt(getChild(node, 4));
        Stmt(getChild(node, 6));
        break;
    case 6: // WHILE LP Exp RP Stmt
        getChild(node, 4)->inh = node->inh;
        Exp(getChild(node, 2));
        Stmt(getChild(node, 4));
        break;
    default:
        break;
    }
}

void DecList(Node* node)
{
    if (node->instruct)
        node->child->instruct = 1;
    node->child->inh = node->inh;
    Dec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // Dec
        break;
    case 2: // Dec COMMA DecList
    {
        if (node->instruct)
            getChild(node, 2)->instruct = 1;
        getChild(node, 2)->inh = node->inh;
        DecList(getChild(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = getChild(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void Dec(Node* node)
{
    if (node->instruct)
        node->child->instruct = 1;
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec ASSIGNOP Exp
        Exp(getChild(node, 2));
        break;
    default:
        break;
    }
}

void Args(Node* node)
{
    Exp(node->child);
    if (node->child->syn) {
        node->syn = newFieldList(node->child->syn->name, node->child->syn->type, NULL);
    } else {
        node->syn = NULL;
    }
    switch (node->no) {
    case 1: // Exp COMMA Args
        Args(getChild(node, 2));
        if (node->syn)
            node->syn->tail = getChild(node, 2)->syn;
        break;
    case 2: // Exp
        break;
    default:
        break;
    }
}


void Exp(Node* node)
{
    TableList* res;
    FieldList* attr;
    Type* type;
    switch (node->no) {
    case 1: // Exp ASSIGNOP Exp
    {
        Exp(node->child);
        
        Exp(getChild(node, 2));
        if (node->child->syn->type->kind == STRUCTVAR) {
            FieldList* p = node->child->syn->type->u.structvar;
            FieldList* q = getChild(node, 2)->syn->type->u.structvar;
            node->syn = node->child->syn;
        } else if (node->child->syn->type->kind == ARRAY) {
            Type* p = node->child->syn->type;
            Type* q = getChild(node, 2)->syn->type;
            node->syn = node->child->syn;
        } else {
            node->syn = node->child->syn;
        }
    } break;
    case 2: // Exp AND Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(INT);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 3: // Exp OR Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(INT);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 4: // Exp RELOP Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(INT);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 5: // Exp PLUS Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(node->child->syn->type->kind);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 6: // Exp MINUS Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(node->child->syn->type->kind);
        node->syn = newFieldList(NULL, type, NULL);
        
        break;
    case 7: // Exp STAR Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(node->child->syn->type->kind);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 8: // Exp DIV Exp
        Exp(node->child);
        Exp(getChild(node, 2));
        type = typewithKind(node->child->syn->type->kind);
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 9: // LP Exp RP
        Exp(getChild(node, 1));
        node->syn = getChild(node, 1)->syn;
        break;
    case 10: // MINUS Exp
        Exp(getChild(node, 1));
        node->syn = getChild(node, 1)->syn;
        break;
    case 11: // NOT Exp
        Exp(getChild(node, 1));
        node->syn = getChild(node, 1)->syn;
        break;
    case 12: // ID LP Args RP
        res = search(node->child->val.type_str);
        Args(getChild(node, 2));
        if (!getChild(node, 2)->syn) {
            node->syn = NULL;
            return;
        }
        node->syn = newFieldList(NULL, res->type->u.function.ret, NULL);
        break;
    case 13: // ID LP RP
        res = search(node->child->val.type_str);
        node->syn = newFieldList(NULL, res->type->u.function.ret, NULL);
        break;
    case 14: // Exp LB Exp RB
    {
        Exp(node->child);
        attr = node->child->syn;
        if (!attr)
            return;
        node->syn = newFieldList(attr->name, attr->type->u.array.elem, NULL);
        
        Exp(getChild(node, 2));
        
    } break;
    case 15: // Exp DOT ID
    {
        Exp(node->child);
        FieldList* attr = node->child->syn;
        if (!attr)
            return;
        for (FieldList* p = attr->type->u.structvar; p; p = p->tail) {
            if (!strcmp(getChild(node, 2)->val.type_str, p->name)) {
                node->syn = p;
                break;
            }
        }

    } break;
    case 16: // ID
        res = search(node->child->val.type_str);
        node->syn = newFieldList(res->name, res->type, NULL);
        break;
    case 17: // INT
    {
        Type* type = typewithKind(INT);
        node->syn = newFieldList(NULL, type, NULL);
    } break;
    case 18: // FLOAT
    {
        Type* type = typewithKind(FLOAT);
        node->syn = newFieldList(NULL, type, NULL);

    } break;
    default:
        break;
    }
}

void Def(Node* node)
{
    // Specifier DecList SEMI
    if (node->instruct) {
        node->child->instruct = 1;
        getChild(node, 1)->instruct = 1;
    }
    node->child->inh = node->inh;
    Specifier(node->child);
    getChild(node, 1)->inh = node->child->syn;
    DecList(getChild(node, 1));
    node->syn = getChild(node, 1)->syn;
}


unsigned hash_pjw(char* name)
{
    unsigned val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~HASHSIZE)
            val = (val ^ (i >> 12)) & HASHSIZE;
    }
    return val;
}

void initTable()
{
    for (int i = 0; i < HASHSIZE + 1; ++i) {
        hashTable[i] = NULL;
    }
    Type* type = typewithKind(INT);
    Type* functype = typewithKind(FUNC);
    functype->u.function.argc = 0;
    functype->u.function.args = NULL;
    functype->u.function.ret = type;
    TableList* item = newTableList("read", functype, 0, NULL, NULL);
    insert(item);
    FieldList* arg = (FieldList*)malloc(sizeof(FieldList));
    arg->name = "";
    arg->tail = NULL;
    arg->type = type;
    functype = typewithKind(FUNC);
    functype->u.function.argc = 1;
    functype->u.function.args = arg;
    functype->u.function.ret = type;
    item = newTableList("write", functype, 0, NULL, NULL);
    insert(item);
}

TableList* search(char* name)
{
    unsigned index = hash_pjw(name);
    for (TableList* node = hashTable[index]; node; node = node->next) {
        if (!strcmp(name, node->name))
            return node;
    }
    return NULL;
}

void insert(TableList* item)
{
    unsigned index = hash_pjw(item->name);
    item->next = hashTable[index];
    hashTable[index] = item;
}

int structcmp(FieldList* s1, FieldList* s2)
{
    int res = 0;
    while (s1 && s2) {
        if (s1 == s2)
            return 1;
        else if (s1->type->kind != s2->type->kind) {
            return 0;
        } else if (s1->type->kind == STRUCTVAR || s1->type->kind == STRUCTURE) {
            res = structcmp(s1->type->u.structvar, s2->type->u.structvar);
            if (!res)
                return 0;
        } else if (s1->type->kind == ARRAY) {
            res = arraycmp(s1->type, s2->type);
            if (!res)
                return 0;
        } else {
            res = 1;
        }
        s1 = s1->tail;
        s2 = s2->tail;
    }
    if (s1 || s2)
        res = 0;
    return res;
}

int arraycmp(Type* t1, Type* t2)
{
    int res;
    while (t1 && t2) {
        if (t1 == t2)
            return 1;
        else if (t1->u.array.elem->kind != t2->u.array.elem->kind) {
            return 0;
        } else if (t1->u.array.elem->kind == STRUCTURE || t1->u.array.elem->kind == STRUCTVAR) {
            res = structcmp(t1->u.array.elem->u.structure, t2->u.array.elem->u.structure);
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

int funccmp(FieldList* arg1, FieldList* arg2)
{
    int res;
    while (arg1 && arg2) {
        if (arg1 == arg2)
            return 1;
        else if (arg1->type->kind != arg2->type->kind) {
            return 0;
        } else if (arg1->type->kind == STRUCTVAR) {
            res = structcmp(arg1->type->u.structvar, arg2->type->u.structvar);
            if (!res)
                return 0;
        } else if (arg1->type->kind == ARRAY) {
            res = arraycmp(arg1->type, arg2->type);
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
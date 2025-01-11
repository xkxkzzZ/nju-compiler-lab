#include "qwq.h"

Node* newNode(char* _name, MyType _type, char* text)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = -1;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    switch (_type) {
    case Int: {
        if (*res->name == 'O')
            sscanf(text, "%ou", &res->val.type_int);
        else if (*res->name == 'H')
            sscanf(text, "%xu", &res->val.type_int);
        else
            res->val.type_int = (unsigned)atoi(text);
        break;
    }
    case Float:
        res->val.type_float = (float)atof(text);
        break;
    case Id:
    case TYpe:
    case Relop:
    case Ter:
        strcpy(res->val.type_str, text);
        break;
    default:
        printf("Wrong Type: %s\n", res->name);
        break;
    }
    return res;
}
Node* newnewNode(char* _name, int _no, MyType _type, int line)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = _no;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    res->line = line;
    return res;
}

void addchild(Node* parent, Node* child)
{
    if (!parent->child) {
        parent->child = child;
    } else {
        Node* p = parent->child;
        while (p->next) {
            p = p->next;
        }
        p->next = child;
    }
    child->parent = parent;
}


Node *getChild(Node *node, int index)
{
    Node* p = node->child;
    while (index-- && p) {
        p = p->next;
    }
    if (!p) {
        printf("Index of child out of range!\n");
        exit(-1);
    }
    return p;
}
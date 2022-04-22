#include "tnode.h"
#include <stdio.h>

Tree newTree(char *type, int num, int numOfChild, ...)
{
    tnode rootNode = (tnode)malloc(sizeof(struct Tnode));
    tnode child;
    if (!rootNode)
    {
        yyerror("malloc tnode error");
        exit(-1);
    }
    rootNode->type = type;
    rootNode->num = num;
    rootNode->leftchild = rootNode->next = NULL;

    va_list param_list;
    va_start(param_list, numOfChild); //初始化参 数列表

    if (numOfChild > 0) //非终结符 union字段为空
    {
        child = va_arg(param_list, tnode);
        if (child->num >= 0)
            IsChild[child->num] = 1;
        rootNode->lineno = child->lineno;
        rootNode->leftchild = child;

        rootNode->content = child->content; //传递第一个孩子节点的语义值
        rootNode->tag = child->tag;
        if (numOfChild >= 2)
        {
            for (int i = 0; i < numOfChild - 1; i++)
            {
                child->next = va_arg(param_list, tnode);
                child = child->next;
                if (child->num >= 0)
                    IsChild[child->num] = 1;
            }
        }
    }
    else
    { //终结符 或 空的语法单元
        rootNode->lineno = va_arg(param_list, int);
        if (!strcmp(type, "INT"))
        {
            rootNode->numtype = "int";
            rootNode->intval = atoi(yytext);
        }
        else if (!strcmp(type, "FLOAT"))
        {
            rootNode->numtype = "float";
            // rootNode->content = yytext;
            rootNode->fltval = atof(yytext);
        }
        else
        {
            char *str = (char *)malloc(strlen(yytext) + 2);
            memset(str, 0, strlen(yytext) + 2);
            strncpy(str, yytext, strlen(yytext) + 1);
            rootNode->content = str;
            rootNode->ID_TYPE = str;
        }
    }
    return rootNode;
}

void Preorder(Tree node, int level)
{
    if (node != NULL)
    {
        for (int i = 0; i < level; i++)
        {
            printf("  ");
        }
        if (node->lineno >= 0) //空语法单元的lineno为-1
        {
            printf("%s", node->type);
            if ((!strcmp(node->type, "ID")) || (!strcmp(node->type, "TYPE")))
            {
                printf(": %s", node->ID_TYPE);
            }
            else if (!strcmp(node->type, "INT"))
            {
                printf(": %d", node->intval);
            }
            else if (!strcmp(node->type, "FLOAT"))
            {
                printf(": %f", node->fltval);
            }
            else
            {
                printf(" (%d)", node->lineno);
            }
        }
        printf("\n");
        Preorder(node->leftchild, level + 1);
        Preorder(node->next, level);
    }
}

/**********************变量符号表操作******************/
void newvar(int num, ...)
{
    va_list valist;
    va_start(valist, num);
    var *res = (var *)malloc(sizeof(var));
    tnode temp = (tnode)malloc(sizeof(tnode));
    if (instruct && LCnum)
    {
        //在结构体中且未闭合
        res->instruct = 1;
        res->structno = structno;
    }
    else
    {
        res->instruct = 0;
        res->structno = -1; // TODO:
    }
    temp = va_arg(valist, tnode);
    res->type = temp->content;
    temp = va_arg(valist, tnode);
    res->name = temp->content;

    vartail->next = res;
    vartail = res;

    //清空varname[]
    // for (int i = 1; i < varnameno; i++)
    //{
    //    var *res = (var *)malloc(sizeof(var));
    //    res->type = res->name = varname[i];
    //    vartail->next = res;
    //    vartail = res;
    //}
    // varnameno = 0;
}
int findvar(tnode val)
{
    var *temp = varhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            if (instruct && LCnum)
            {
                if (!temp->instruct)
                {
                    printf("Error type X at line %d:Struct field and Variable use the same name.\n", yylineno);
                }
                else if (temp->instruct && temp->structno != structno)
                {
                    printf("Error type Y at line %d:Different struct use the same field name\n", yylineno);
                }
                else
                {
                    return 1; //同一结构体域名重复
                }
            }
            else //全局变量
            {
                if (temp->instruct)
                {
                    printf("Error type X at line %d:Struct field and Variable use the same name.\n ", yylineno);
                }
                else
                {
                    return 1; //变量重复定义
                }
            } // if(instruct)
        }     // if(!strcmp)
        temp = temp->next;
    } // while
    return 0;
}
//由变量名获取变量信息,返回值为name在列表中的位置序号，pos为传出参数，指向变量信息
int getvarstr(char *name, var *pos)
{
    var *temp = varhead->next;
    int i = 0;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, name))
        {
            pos = temp;
            return i;
        } // if(!strcmp)
        temp = temp->next;
        i++;
    } // while
    return -1;
}

char *typevar(tnode val)
{
    var *temp = varhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            return temp->type;
        }
        temp = temp->next;
    }
    return NULL;
}
int checkleft(tnode val)
{
    tnode temp = val->leftchild;
    if (temp == NULL)
    {
        return 0; //失败
    }
    tnode firstchild = val->leftchild;
    int childnum = 0;
    while (temp != NULL)
    {
        childnum++;
        temp = temp->next;
    }
    if (childnum == 1 && !strcmp(firstchild->type, "ID"))
    {
        return 1;
    }
    else if (childnum == 4 && !strcmp(firstchild->type, "Exp") && !strcmp(firstchild->next->type, "LB") && !strcmp(firstchild->next->next->type, "Exp") && !strcmp(firstchild->next->next->next->type, "RB"))
    {
        return 1;
    }
    else if (childnum == 3 && !strcmp(firstchild->type, "Exp") && !strcmp(firstchild->next->type, "DOT") && !strcmp(firstchild->next->next->type, "ID"))
    {
        return 1;
    }
    else
        return 0;
}

/***************函数符号表******************/
void newfunc(int num, ...)
{
    int i;
    va_list valist;
    va_start(valist, num);
    tnode temp = (tnode)malloc(sizeof(struct Tnode));
    switch (num)
    {
    case 1:
        if (instruct && LCnum)
        {
            functail->instruct = 1;
            functail->structno = structno;
        }
        else
        {
            functail->instruct = 0;
            functail->structno = 0;
        }
        temp = va_arg(valist, tnode);
        functail->rtype = temp->content; //返回值类型
        functail->type = temp->numtype;  // 语法单元的类型
        for (i = 0; i < rnum; i++)
        {
            if (rtype[i] == NULL || strcmp(rtype[i], functail->rtype))
            {
                printf("Error type 8 at line %d:Function return type error(%s->%s)\n", yylineno, functail->rtype, rtype[i]);
            }
        }
        functail->tag = 1;
        func *new = (func *)malloc(sizeof(func));
        functail->next = new;
        functail = new;
        break;
    case 2:
        temp = va_arg(valist, tnode);
        functail->name = temp->content; //函数名
        temp = va_arg(valist, tnode);
        functail->va_num = 0;
        getdeftype(temp);
        break;
    default:
        break;
    }
}
void getdeftype(tnode val)
{
    if (val != NULL)
    {
        if (!strcmp(val->type, "ParamDec"))
        {
            functail->va_type[functail->va_num] = val->leftchild->content;
            functail->va_num++;
            return;
        }
        tnode child = val->leftchild;
        while (child != NULL)
        {
            getdeftype(child);
            child = child->next;
        }
    }
    return;
}
void getreatype(tnode val)
{
    if (val != NULL)
    {
        if (!strcmp(val->type, "Exp"))
        {
            va_type[va_num] = val->numtype;
            va_num++;
            return;
        }
        tnode child = val->leftchild;
        while (child != NULL)
        {
            getreatype(child);
            child = child->next;
        }
    }
}
void getrtype(tnode val) //实际返回值类型
{
    rtype[rnum] = val->numtype;
    rnum++;
}
int checkrtype(tnode ID, tnode Args)
{
    va_num = 0;
    getreatype(Args);
    func *temp = funchead->next;
    while (temp != NULL && temp->name != NULL && temp->tag == 1)
    {
        if (!strcmp(temp->name, ID->content))
        {
            break;
        }
        temp = temp->next;
    }
    if (va_num != temp->va_num)
    {
        return 1;
    }
    for (int i = 0; i < temp->va_num; i++)
    {
        if (temp->va_type[i] == NULL || va_type[i] == NULL || strcmp(temp->va_type[i], va_type[i]) != 0)
        {
            return 1;
        }
    }
    return 0; //成功
}
int findfunc(tnode val)
{ //函数是否已定义
    func *temp = funchead->next;
    while (temp != NULL && temp->name != NULL && temp->tag == 1)
    {
        if (!strcmp(temp->name, val->content))
        {
            if (instruct && LCnum)
            {
                if (!temp->instruct)
                {
                    printf("Error type X at line %d:Struct field and Variable use the same name.\n", yylineno);
                }
                else if (temp->instruct && temp->structno != structno)
                {
                    printf("Error type Y at line %d:Different struct use the same field name\n", yylineno);
                }
                else
                {
                    //同一结构体中域名重复
                    printf("Error type 15 at line %d:Redefined field", yylineno); // TODO:
                    return 1;
                } // if(!temp->instruct)
            }     // if(instruct)
            else
            {
                if (temp->instruct)
                {
                    printf("Error type X at line %d:Struct field and Variable use the same name.\n", yylineno);
                }
                else
                {
                    return 1; // TODO:
                }
            } // if(instruct)
        }     // if(!strcmp)
        temp = temp->next;
    } // while
    return 0;
}
func *getfunc(tnode val)
{
    func *temp = funchead->next;
    while (temp != NULL && temp->name != NULL && temp->tag == 1)
    {
        if (!strcmp(temp->name, val->content))
        {
            return temp;
        } // if(!strcmp)
        temp = temp->next;
    } // while
    return 0;
}
char *typefunc(tnode val)
{
    func *temp = funchead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            char *args = (char *)malloc(sizeof(char) * 100);
            memset(args, 0, sizeof(char) * 100);
            int i = 0;
            char *pi = args;
            while (i < temp->va_num && pi < args + 99)
            {
                memcpy(pi, temp->va_type[i], strlen(temp->va_type));
                pi += strlen(temp->va_type[i]);
                memcpy(pi, ",", 1);
                pi += 1;
                i++;
            }
            *(pi - 1) = 0;
            return args; //返回:"arg1,arg2,arg3"
        }
        temp = temp->next;
    }
    return NULL;
}
int numfunc(tnode val)
{
    func *temp = funchead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            return temp->va_num;
        }
        temp = temp->next;
    }
    return 0;
}
//检查函数是否重定义
int checkmultifuncstr(char *str)
{
    func *pos = funchead->next;
    while (pos != NULL)
    {
        if (!strcmp(pos->name, str))
        {
            return 0; //已定义
        }
        pos = pos->next;
    }
    return 1;
}

/************************数组符号表*******************/
void newarray(int num, ...)
{
    va_list valist;
    va_start(valist, num);
    array *res = (array *)malloc(sizeof(array));
    tnode temp = (tnode)malloc(sizeof(struct Tnode));
    if (instruct && LCnum)
    {
        res->instruct = 1;
        res->structno = structno;
    }
    else
    {
        res->instruct = 0;
        res->structno = 0; // TODO:
    }
    temp = va_arg(valist, tnode);
    res->type = temp->content;
    temp = va_arg(valist, tnode);
    res->name = temp->content;
    arraytail->next = res;
    arraytail = res;
}
int findarray(tnode val)
{
    array *temp = arrayhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}
char *typearray(tnode val)
{
    array *temp = arrayhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            return temp->type;
        }
        temp = temp->next;
    }
    return NULL;
}
/******************结构体符号表***************/
void newstruct(int num, ...)
{
    va_list valist;
    va_start(valist, num);
    struct_ *res = (struct_ *)malloc(sizeof(struct_));
    tnode temp = va_arg(valist, tnode);
    res->name = temp->content;
    structtail->next = res;
    structtail = res;
}
int findstruct(tnode val) //检查结构体是否已定义 content
{
    struct_ *temp = (struct_ *)malloc(sizeof(struct_));
    temp = structhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, val->content))
        {
            return 1;
        }
        temp = temp->next;
    }
    if (findvar(val) == 1) //结构体名称不和变量名冲突
    {
        return 0;
    }
    return 0;
}
int findstructstr(char *str)
{
    if (str == NULL)
        return;
    struct_ *temp = (struct_ *)malloc(sizeof(struct_));
    temp = structhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, str))
        {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}
struct_ *getstructstr(char *str)
{
    if (str == NULL)
        return;
    struct_ *temp = (struct_ *)malloc(sizeof(struct_));
    temp = structhead->next;
    while (temp != NULL)
    {
        if (!strcmp(temp->name, str))
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
void addstructfield(struct_ *structpos, tnode node)
{
    if (node != NULL)
    {
        if (!strcmp(node->type, "ID"))
        {
            //加入新field
            fieldlist temp = (fieldlist)malloc(sizeof(struct fieldlist_));
            if (structpos->fields == NULL)
            {
                structpos->fields = temp;
                fieldtail = structpos->fields;
            }
            else
            {
                fieldtail->next = temp;
                fieldtail = temp;
            }
            fieldtail->name = node->content;
            fieldtail->next = NULL;
        }
        tnode child = node->leftchild;
        while (child != NULL)
        {
            addstructfield(structpos, child);
            child = child->next;
        }
    }
}
int checkstructfield(struct_ *structpos, char *name)
{ //检查域是否存在
    if (structpos == NULL)
        return 0; //不是结构体
    fieldlist tail = structpos->fields;
    while (tail != NULL)
    {
        if (!strcmp(tail->name, name))
        {
            return 1;
        }
        tail = tail->next;
    }
    return 0;
}
//检查结构体中是否有重复定义的域
char *checkmultifield(struct_ *structpos)
{
    fieldlist tail = structpos->fields;
    while (tail != NULL)
    {
        fieldlist pos = tail->next;
        while (pos != NULL)
        {
            if (!strcmp(tail->name, pos->name))
            {
                return tail->name; //重复定义
            }
            pos = pos->next;
        }
        tail = tail->next;
    }
    return NULL;
}

/*************************中间代码生成*********************/
void init_tempvar_lable()
{
    for (int i = 0; i < MAX_NUM; i++)
    {
        tempvar[i] = -1;
        lables[i] = -1;
    }
}
Operand new_tempvar()
{
    if (tempvarnum >= MAX_NUM)
        return NULL;
    Operand res = new_Operand();
    res->kind = TEMPVAR;
    res->operand.tempvar = tempvarnum; // TODO:
    tempvar[tempvarnum++] = res;
    return res;
}
Operand new_lable()
{
    if (lablesnum > MAX_NUM)
        return NULL;
    Operand res = new_Operand();
    res->kind = LABLE;
    res->operand.lable = lablesnum; // TODO:
    lables[lablesnum++] = res;
    return res;
}
Operand new_Operand()
{
    Operand res = (Operand)malloc(sizeof(OperandStru));
    res->value = -10000; // TODO:
    return res;
}
Operand new_Variable(char *name)
{
    Operand res = new_Operand();
    res->kind = VARIABLE;
    res->operand.name = name;
    return res;
}
Operand new_Const(int value)
{
    Operand res = new_Operand();
    res->kind = CONSTANT;
    res->operand.value = value;
    res->value = value;
    return res;
}
InterCode new_Code()
{
    InterCode res = (InterCode)malloc(sizeof(InterCodeStru));
    res->kind = _NULL;
    res->prev = NULL;
    res->next = NULL;
    return res;
}

// lable声明
InterCode new_lable_Code(Operand lable)
{
    InterCode res = new_Code();
    res->kind = _LABLE;
    res->operands.var = lable;
    return res;
}
InterCode new_goto_Code(Operand lable)
{
    InterCode res = new_Code();
    res->kind = _GOTO;
    res->operands.jump.lable = lable;
    return res;
}
InterCode new_assign_Code(Operand left, Operand right)
{
    left->value = right->value; // TODO:
    InterCode res = new_Code();
    res->kind = _ASSIGN;
    res->operands.assign.left = left;
    res->operands.assign.right = right;
    return res;
}
void print_Code(InterCode code)
{
    if (code == NULL)
    {
        perror("Error0 in print_Code()\n");
        return;
    }
    switch (code->kind)
    {
    case _NULL:
        break;
    case _LABLE:
        printf("LABLE ");
        print_Operand(code->operands.var);
        printf(" :");
        break;
    case _FUNCTION:
        printf("FUNCTION ");
        print_Operand(code->operands.var);
        printf(" :");
        break;
    case _ASSIGN:
        print_Operand(code->operands.assign.left);
        printf(" := ");
        print_Operand(code->operands.assign.right);
        break;
    case _ADD:
        print_Operand(code->operands.binop.result);
        printf(" := ");
        print_Operand(code->operands.binop.op1);
        printf(" + ");
        print_Operand(code->operands.binop.op2);
        break;
    case _SUB:
        print_Operand(code->operands.binop.result);
        printf(" := ");
        print_Operand(code->operands.binop.op1);
        printf(" - ");
        print_Operand(code->operands.binop.op2);
        break;
    case _MUL:
        print_Operand(code->operands.binop.result);
        printf(" := ");
        print_Operand(code->operands.binop.op1);
        printf(" * ");
        print_Operand(code->operands.binop.op2);
        break;
    case _DIV:
        print_Operand(code->operands.binop.result);
        printf(" := ");
        print_Operand(code->operands.binop.op1);
        printf(" / ");
        print_Operand(code->operands.binop.op2);
        break;
    case _GOTO:
        printf("GOTO ");
        print_Operand(code->operands.jump.lable);
        break;
    case _IFGOTO:
        printf("IF ");
        print_Operand(code->operands.jump.op1);
        printf(" %s ", code->operands.jump.relop);
        print_Operand(code->operands.jump.op2);
        printf(" GOTO ");
        print_Operand(code->operands.jump.lable);
        break;
    case _RETURN:
        printf("RETURN ");
        print_Operand(code->operands.var);
        break;
    case _ARG:
        printf("ARG ");
        print_Operand(code->operands.var);
        break;
    case _CALL:
        if (code->operands.assign.left == NULL)
            printf("CALL ");
        else
        {
            print_Operand(code->operands.assign.left);
            printf(" := CALL ");
        }
        print_Operand(code->operands.assign.right);
        break;
    case _PARAM:
        printf("PARAM ");
        print_Operand(code->operands.var);
        break;
    case _READ:
        printf("READ ");
        print_Operand(code->operands.var);
        break;
    case _WRITE:
        printf("WRITE ");
        print_Operand(code->operands.var);
        break;
    default:
        printf("Error1 in print_Code()\n");
        break;
    }
    if (code->kind != NULL)
        printf("\n");
}
void print_Operand(Operand op)
{
    if (op == NULL)
    {
        printf("Error0 in print_Operand()\n");
        return;
    }
    switch (op->kind)
    {
    case VARIABLE:
    case FUNC:
        printf("%s", op->operand.name);
        break;
    // case VARIABLE:
    //     printf("v%d", op->operand.tempvar);
    case TEMPVAR:
        printf("t%d", op->operand.tempvar);
        break;
    case LABLE:
        printf("lable%d", op->operand.lable);
        break;
    case CONSTANT:
        printf("#%d", op->operand.value);
        break;
    case ADDRESS:
        printf("&%s", op->operand.name);
        break;
    case VALUE:
        printf("*%s", op->operand.name);
        break;
    default:
        printf("Error1 in print_Operand()\n");
        break;
    }
}
void print_Codes(InterCode codes)
{
    // printf("\n*****************************\n");
    InterCode temp = codes;
    while (temp)
    {
        print_Code(temp);
        temp = temp->next;
    }
    // printf("***********************************\n");
}
InterCode add_Codes(int num, ...)
{
    va_list list;
    va_start(list, num);
    InterCode temp;
    InterCode code = va_arg(list, InterCode);
    CodesTail->next = code;
    code->prev = CodesTail;
    CodesTail = code;
    for (int i = 1; i < num; i++)
    {
        temp = va_arg(list, InterCode);
        CodesTail->next = temp;
        temp->prev = CodesTail;
        CodesTail = temp;
    }
    return code; //第一句
}
InterCode translate_Program(tnode Program)
{
    return translate_ExtDefList(Program->leftchild);
}
InterCode translate_ExtDefList(tnode ExtDefList)
{
    if (ExtDefList->leftchild != NULL && ExtDefList->leftchild->lineno != -1)
    {
        InterCode code1 = translate_ExtDef(ExtDefList->leftchild);
        InterCode code2 = translate_ExtDefList(ExtDefList->leftchild->next);
        // add_Codes(2, code1, code2);
        return code1; // TODO:
    }
    return new_Code();
}
InterCode translate_ExtDef(tnode ExtDef)
{
    if (ExtDef->leftchild != NULL)
    {
        if (!strcmp(ExtDef->leftchild->next->type, "ExtDecList"))
        {
        }
        else if (!strcmp(ExtDef->leftchild->next->type, "SEMI"))
        {
        }
        else if (!strcmp(ExtDef->leftchild->next->type, "FunDec"))
        {
            InterCode code1 = translate_FunDec(ExtDef->leftchild->next);
            InterCode code2 = translate_CompSt(ExtDef->leftchild->next->next);
            // add_Codes(2, code1, code2);
            return code1;
        }
    }
    return new_Code();
}
//函数头的定义
InterCode translate_FunDec(tnode FunDec)
{
    if (FunDec->leftchild != NULL)
    {
        Operand function = new_Variable(FunDec->leftchild->content);
        InterCode code1 = new_Code();
        code1->kind = _FUNCTION;
        code1->operands.var = function;
        add_Codes(1, code1);
        if (!strcmp(FunDec->leftchild->next->next->type, "VarList"))
        {
            InterCode code2 = translate_VarList(FunDec->leftchild->next->next);
        }
        else if (!strcmp(FunDec->leftchild->next->next->type, "RP"))
        {
        }
        return code1;
    }
    return new_Code();
}
InterCode translate_VarList(tnode VarList)
{
    if (VarList->leftchild != NULL)
    {
        if (VarList->leftchild->next != NULL)
        {
            InterCode code1 = translate_ParamDec(VarList->leftchild);
            InterCode code2 = translate_VarList(VarList->leftchild->next->next);
            return code1;
        }
        else
        {
            return translate_ParamDec(VarList->leftchild);
        }
    }
    return new_Code();
}
InterCode translate_ParamDec(tnode ParamDec)
{
    if (ParamDec->leftchild != NULL)
    {
        // VarDec->ID
        tnode VarDec = ParamDec->leftchild->next;
        InterCode code1 = new_Code();
        code1->kind = _PARAM;
        char *varname = getvarnamestr(VarDec->content);
        if (varname == NULL)
        {
            printf("Error- in translate_ParamDec() 1\n");
            return NULL;
        }
        code1->operands.var = new_Variable(varname);
        // code1->operands.var = new_tempvar();
        add_Codes(1, code1); //叶节点加入
        return code1;
    }
    return new_Code();
}

//函数体
InterCode translate_CompSt(tnode CompSt)
{
    if (CompSt->leftchild != NULL)
    {
        InterCode code1 = translate_DefList(CompSt->leftchild->next);
        InterCode code2 = translate_StmtList(CompSt->leftchild->next->next);
        return code1;
    }
    return new_Code();
}
InterCode translate_StmtList(tnode StmtList)
{
    if (StmtList->leftchild != NULL && StmtList->leftchild->lineno != -1)
    {
        InterCode code1 = translate_Stmt(StmtList->leftchild);
        InterCode code2 = translate_StmtList(StmtList->leftchild->next);
        return code1;
    }
    return new_Code();
}

//语句
InterCode translate_Stmt(tnode Stmt)
{
    if (Stmt->leftchild != NULL)
    {
        if (!strcmp(Stmt->leftchild->type, "Exp"))
        {
            return translate_Exp(Stmt->leftchild, NULL);
        }
        else if (!strcmp(Stmt->leftchild->type, "CompSt"))
        {
            return translate_CompSt(Stmt->leftchild);
        }
        else if (!strcmp(Stmt->leftchild->type, "RETURN"))
        {
            Operand op = get_Operand(Stmt->leftchild->next);
            if (op == NULL) //操作数没有声明过
            {
                Operand t1 = new_tempvar();
                InterCode code1 = translate_Exp(Stmt->leftchild->next, t1);
                InterCode code2 = new_Code();
                code2->kind = _RETURN;
                code2->operands.var = t1;
                add_Codes(1, code2);
                return code1;
            }
            else
            {
                InterCode code1 = translate_Exp(Stmt->leftchild->next, op);
                InterCode code2 = new_Code();
                code2->kind = _RETURN;
                code2->operands.var = op;
                add_Codes(1, code2);
                return code1;
            }
        }
        else if (!strcmp(Stmt->leftchild->type, "IF"))
        {
            tnode ExpChild = Stmt->leftchild->next->next;
            tnode StmtChild = ExpChild->next->next;
            Operand lable1 = new_lable();
            Operand lable2 = new_lable();
            InterCode code1 = translate_Cond(ExpChild, lable1, lable2);
            add_Codes(1, new_lable_Code(lable1));
            InterCode code2 = translate_Stmt(StmtChild);

            if (StmtChild->next == NULL) //没有ELSE
            {
                add_Codes(1, new_lable_Code(lable2));
            }
            else
            {
                tnode StmtChild2 = StmtChild->next->next;
                Operand lable3 = new_lable();
                add_Codes(1, new_goto_Code(lable3));
                add_Codes(1, new_lable_Code(lable2));
                InterCode code3 = translate_Stmt(StmtChild2);
                add_Codes(1, new_lable_Code(lable3));
            }
            return code1;
        }
        else if (!strcmp(Stmt->leftchild->type, "WHILE"))
        {
            tnode ExpChild = Stmt->leftchild->next->next;
            tnode StmtChild = ExpChild->next->next;
            Operand lable1 = new_lable();
            Operand lable2 = new_lable();
            Operand lable3 = new_lable();
            add_Codes(1, new_lable_Code(lable1));
            InterCode code1 = translate_Cond(ExpChild, lable1, lable2);
            add_Codes(1, new_lable_Code(lable2));
            InterCode code2 = translate_Stmt(StmtChild);
            add_Codes(1, new_goto_Code(lable1));
            add_Codes(1, new_lable_Code(lable3));
            return code1;
        }
        return new_Code();
    }
}
//变量声明
InterCode translate_DefList(tnode DefList)
{
    if (DefList->leftchild != NULL && DefList->leftchild->lineno != -1)
    {
        InterCode code1 = translate_Def(DefList->leftchild);
        InterCode code2 = translate_DefList(DefList->leftchild->next);
        return code1;
    }
    return new_Code();
}
InterCode translate_Def(tnode Def)
{
    return translate_DecList(Def->leftchild->next);
}
InterCode translate_DecList(tnode DecList)
{
    tnode DecChild = DecList->leftchild;
    if (DecChild != NULL)
    {
        if (DecChild->next != NULL)
        {
            InterCode code1 = translate_Dec(DecChild);
            InterCode code2 = translate_DecList(DecChild->next->next);
            return code1;
        }
        else
        {
            return translate_Dec(DecChild);
        }
    }
    return new_Code();
}
InterCode translate_Dec(tnode Dec)
{
    tnode VarDecChild = Dec->leftchild;
    if (VarDecChild->next != NULL)
    {
        // VarDec ASSIGNOP Exp
        char *varname = getvarnamestr(VarDecChild->content);
        if (varname == NULL)
        {
            printf("Error- in translate_Dec() 1\n");
            return NULL;
        }
        Operand vari = new_Variable(varname);
        Operand t1 = new_tempvar();
        InterCode code1 = translate_Exp(VarDecChild->next->next, t1);
        InterCode code2 = new_assign_Code(vari, t1);
        add_Codes(1, code2);
    }
    return new_Code();
}

//基本表达式
InterCode translate_Exp(tnode Exp, Operand place)
{
    int isCond = 0; //是否为条件表达式
    tnode Exp1 = Exp->leftchild;
    tnode op = Exp->leftchild->next;
    tnode Exp2 = NULL;
    if (op != NULL)
        Exp2 = op->next;
    if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "Exp"))
    {
        if (!strcmp(op->type, "ASSIGNOP"))
        {
            // Exp1->ID
            if (Exp1->leftchild->next == NULL && !strcmp(Exp1->leftchild->type, "ID"))
            {
                char *varname = getvarnamestr(Exp1->leftchild->content);
                if (varname == NULL)
                {
                    printf("Error- in translate_Exp() %d\n", Exp->lineno);
                    return NULL;
                }
                Operand vari = new_Variable(varname);
                // Operand existOp = get_Operand(Exp2);
                Operand t1 = new_tempvar();
                InterCode code1 = translate_Exp(Exp2, t1); // PLUS
                add_Codes(1, new_assign_Code(vari, t1));
                if (place == NULL)
                {
                    return code1;
                }
                else
                {
                    InterCode code3 = new_assign_Code(place, vari);
                    add_Codes(1, code3);
                    return code1;
                }
            }
        }
        else if (!strcmp(op->type, "MINUS"))
        {
            Operand t1 = new_tempvar();
            Operand t2 = new_tempvar();
            InterCode code1 = translate_Exp(Exp1, t1);
            InterCode code2 = translate_Exp(Exp2, t2);
            InterCode code3 = new_Code();
            code3->kind = _SUB;
            code3->operands.binop.result = place;
            code3->operands.binop.op1 = t1;
            code3->operands.binop.op2 = t2;
            add_Codes(1, code3);
            return code1;
        }
        else if (!strcmp(op->type, "PLUS"))
        {
            Operand t1 = new_tempvar();
            Operand t2 = new_tempvar();
            InterCode code1 = translate_Exp(Exp1, t1);
            InterCode code2 = translate_Exp(Exp2, t2);
            InterCode code3 = new_Code();
            code3->kind = _ADD;
            code3->operands.binop.result = place;
            code3->operands.binop.op1 = t1;
            code3->operands.binop.op2 = t2;
            add_Codes(1, code3);
            return code1;
        }
        else if (!strcmp(op->type, "STAR"))
        {
            Operand t1 = new_tempvar();
            Operand t2 = new_tempvar();
            InterCode code1 = translate_Exp(Exp1, t1);
            InterCode code2 = translate_Exp(Exp2, t2);
            InterCode code3 = new_Code();
            code3->kind = _MUL;
            code3->operands.binop.result = place;
            code3->operands.binop.op1 = t1;
            code3->operands.binop.op2 = t2;
            add_Codes(1, code3);
            return code1;
        }
        else if (!strcmp(op->type, "DIV"))
        {
            Operand t1 = new_tempvar();
            Operand t2 = new_tempvar();
            InterCode code1 = translate_Exp(Exp1, t1);
            InterCode code2 = translate_Exp(Exp2, t2);
            InterCode code3 = new_Code();
            code3->kind = _DIV;
            code3->operands.binop.result = place;
            code3->operands.binop.op1 = t1;
            code3->operands.binop.op2 = t2;
            add_Codes(1, code3);
            return code1;
        }
        else if (!strcmp(op->type, "AND"))
        {
            isCond = 1;
        }
        else if (!strcmp(op->type, "OR"))
        {
            isCond = 1;
        }
        else if (!strcmp(op->type, "RELOP"))
        {
            isCond = 1;
        }
        else if (!strcmp(op->type, "LB"))
        { //数组操作
        }
        else if (!strcmp(op->type, "DOT"))
        { //结构体操作
        }
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "LP"))
    {
        return translate_Exp(Exp->leftchild->next, place);
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "MINUS"))
    {
        Operand t1 = new_tempvar();
        InterCode code1 = translate_Exp(Exp1->next, t1);
        InterCode code2 = new_Code();
        code2->kind = _SUB;
        code2->operands.binop.result = place;
        code2->operands.binop.op1 = new_Const(0);
        code2->operands.binop.op2 = t1;
        add_Codes(1, code2);
        return code1;
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "NOT"))
    {
        isCond = 1;
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "ID"))
    {
        if (Exp->leftchild->next != NULL)
        {
            tnode Args = Exp->leftchild->next->next;
            if (Args->next == NULL) // ID LP RP
            {
                Operand func = new_Operand();
                func->kind = FUNC;
                func->operand.name = Exp->leftchild->content;
                if (!strcmp(func->operand.name, "read"))
                {
                    InterCode code = new_Code();
                    code->kind = _READ;
                    code->operands.var = place;
                    add_Codes(1, code);
                    return code;
                }
                else
                {
                    InterCode code = new_Code();
                    code->kind = _CALL;
                    code->operands.assign.left = place;
                    code->operands.assign.right = func;
                    add_Codes(1, code);
                    return code;
                }
            }    // if args
            else // ID LP Args RP
            {
                Operand func = new_Operand();
                func->kind = FUNC;
                func->operand.name = Exp->leftchild->content;
                ArgList arg_list = (ArgList)malloc(sizeof(ArgListStru));
                arg_list->num = 0;
                InterCode code1 = translate_Args(Args, arg_list);
                InterCode code2, code3;
                if (!strcmp(func->operand.name, "write"))
                {
                    code2 = new_Code();
                    code2->kind = _WRITE;
                    code2->operands.var = (arg_list->list)[0];
                    add_Codes(1, code2);
                }
                else
                {
                    for (int i = 0; i < arg_list->num; i++)
                    {
                        code2 = new_Code();
                        code2->kind = _ARG;
                        code2->operands.var = (arg_list->list)[i];
                        add_Codes(1, code2);
                    }
                    code3 = new_Code();
                    code3->kind = _CALL;
                    code3->operands.assign.left = place;
                    code3->operands.assign.right = func;
                    add_Codes(1, code3);
                    return code1;
                } // if write
            }     // if have args
        }         // if ID->next!=NULL
        else      // Exp->ID
        {
            // TODO:u
            place->kind = VARIABLE;
            place->operand.name = getvarnamestr(Exp->leftchild->content);
        }
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "INT"))
    {
        Operand value = new_Const(Exp->leftchild->intval);
        InterCode code = new_assign_Code(place, value);
        add_Codes(1, code);
        return code;
    }
    else if (Exp->leftchild != NULL && !strcmp(Exp->leftchild->type, "FLOAT"))
    {
        Operand value = new_Const((int)Exp->intval); // TODO:
        InterCode code = new_assign_Code(place, value);
        add_Codes(1, code);
        return code;
    }
    else
    {
        printf("Error0 in translate_Exp()\n");
    }
    if (isCond)
    {
        Operand lable1 = new_lable();
        Operand lable2 = new_lable();
        InterCode code0 = new_assign_Code(place, new_Const(0));
        add_Codes(1, code0);
        InterCode code1 = translate_Cond(Exp, lable1, lable2);
        add_Codes(2, new_lable_Code(lable1), new_assign_Code(place, new_Const(1)));
        add_Codes(new_lable_Code(lable2));
        return code0;
    }
    return new_Code();
}
InterCode translate_Cond(tnode Exp, Operand lable_true, Operand lable_false)
{
    if (Exp->leftchild == NULL)
        return new_Code();
    tnode relop = Exp->leftchild->next;
    if (!strcmp(Exp->leftchild->type, "Exp"))
    {
        if (relop != NULL && !strcmp(relop->type, "RELOP"))
        {
            InterCode code3 = new_Code();
            code3->kind = _IFGOTO;
            code3->operands.jump.lable = lable_true;
            code3->operands.jump.relop = relop->content;
            // Operand t1 = new_tempvar(); // TODO:u 查看Exp->ID 的content是否在变量列表中
            Operand t1;
            Operand t2;
            // Exp->ID
            if (!strcmp(Exp->leftchild->leftchild->type, "ID") && Exp->leftchild->leftchild->next == NULL)
                t1 = new_Variable(getvarnamestr(Exp->leftchild->leftchild->content));
            else
                t1 = new_tempvar();
            if (!strcmp(relop->next->leftchild->type, "ID") && relop->next->leftchild->next == NULL)
                t2 = new_Variable(getvarnamestr(relop->next->leftchild->content));
            else
                t2 = new_tempvar();
            InterCode code1 = translate_Exp(Exp->leftchild, t1);
            InterCode code2 = translate_Exp(relop->next, t2);
            code3->operands.jump.op1 = t1;
            code3->operands.jump.op2 = t2;
            add_Codes(2, code3, new_goto_Code(lable_false));
            return code1;
        }
        else
        {
            printf("Error0 in translate_Cond()\n");
        }
    }
    else if (!strcmp(Exp->leftchild->type, "NOT"))
    {
        return translate_Cond(Exp->leftchild, lable_false, lable_true);
    }
    else if (!strcmp(relop->content, "AND"))
    {
        Operand lable1 = new_lable();
        InterCode code1 = translate_Cond(Exp->leftchild, lable1, lable_false);
        add_Codes(1, new_lable_Code(lable1));
        InterCode code2 = translate_Cond(relop->next, lable_true, lable_false);
        return code1;
    }
    else if (!strcmp(relop->content, "OR"))
    {
        Operand lable1 = new_lable();
        InterCode code1 = translate_Cond(Exp->leftchild, lable_true, lable1);
        add_Codes(1, new_lable_Code(lable1));
        InterCode code2 = translate_Cond(relop->next, lable_true, lable_false);
        return code1;
    }
    else
    {
        Operand t1 = new_tempvar();
        InterCode code1 = translate_Exp(Exp, t1);
        InterCode code2 = new_Code();
        char *relopchr = "!=";
        code2->kind = _IFGOTO;
        code2->operands.jump.lable = lable_true;
        code2->operands.jump.relop = relopchr;
        code2->operands.jump.op1 = t1;
        code2->operands.jump.op2 = new_Const(0);
        add_Codes(2, code2, new_goto_Code(lable_false));
        return code1;
    }
}

InterCode translate_Args(tnode Args, ArgList arg_list)
{
    tnode Exp1 = Args->leftchild;
    if (Exp1 == NULL)
    {
        printf("Error0 in translate_Args()\n");
        return new_Code();
    }
    if (Exp1->next == NULL) // Exp
    {
        Operand t1 = new_tempvar();
        InterCode code1 = translate_Exp(Exp1, t1);
        arg_list->list[arg_list->num] = t1;
        arg_list->num++;
        return code1;
    }
    else
    {
        Operand t1 = new_tempvar();
        InterCode code1 = translate_Exp(Exp1, t1);
        arg_list->list[arg_list->num] = t1;
        arg_list->num++;
        InterCode code2 = translate_Args(Exp1->next->next, arg_list);
    }
    return new_Code();
}
Operand get_Operand(tnode node)
{
    return NULL;
}

//传入变量名，返回v1 v2...，若该变量未定义则返回NULL
char *getvarnamestr(char *name)
{
    var *pos = NULL;
    int num = getvarstr(name, pos);
    if (num < 0)
    {
        //该变量未定义TODO:
        var *res = (var *)malloc(sizeof(var));
        res->instruct = 0;
        res->structno = -1;
        res->type = res->name = name;
        vartail->next = res;
        vartail = res;
        return getvarnamestr(name);
    }
    else
    {
        char *v = (char *)malloc(sizeof(10));
        memset(v, 0, 10);
        memcpy(v, "v", 1);
        sprintf(v + 1, "%d", num);
        return v;
    }
}

int Error = 0;
void yyerror(char *msg)
{
    Error = 1;
    fprintf(stderr, "Error Type B at Line %d : %s \'%s\'\n", yylineno, msg, yytext);
}
extern int yydebug;
int main(int argc, char **argv)
{
    if (argc <= 1)
        return 0;
    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        perror(argv[1]);
        return 1;
    }
    nodeNum = 0;
    memset(nodeList, NULL, sizeof(tnode) * LISTSIZE);
    memset(IsChild, 0, sizeof(int) * LISTSIZE);
    Error = 0;

    instruct = 0; // TODO:
    LCnum = 0;
    structno = 0;
    //初始化符号表
    varhead = (var *)malloc(sizeof(var));
    vartail = varhead;
    funchead = (func *)malloc(sizeof(func));
    functail = (func *)malloc(sizeof(func));
    funchead->next = functail;
    functail->va_num = 0;
    arrayhead = (array *)malloc(sizeof(array));
    arraytail = arrayhead;
    structhead = (struct_ *)malloc(sizeof(struct_));
    structtail = structhead;
    rnum = 0;
    // structnum = 0;
    varnameno = 0;

    //中间代码生成部分
    tempvarnum = 0;
    lablesnum = 0;
    init_tempvar_lable();
    //添加read、write函数
    char *read = "read";
    char *write = "write";
    char *typeInt = "int";
    functail->tag = 1;
    functail->name = read;
    functail->type = typeInt;
    functail->rtype = typeInt;
    functail->va_num = 0;
    functail->instruct = 0;
    functail->structno = 0;

    func *new = (func *)malloc(sizeof(func));
    functail->next = new;
    functail = new;

    functail->tag = 1;
    functail->name = write;
    functail->va_num = 1;
    (functail->va_type)[0] = typeInt;
    functail->instruct = 0;
    functail->structno = 0;

    new = (func *)malloc(sizeof(func));
    functail->next = new;
    functail = new;
    //初始化中间代码列表
    CodesHead = (InterCode)malloc(sizeof(InterCodeStru));
    CodesTail = CodesHead;

    yyrestart(file);
    // yydebug = 1;
    yyparse();
    fclose(file);

    if (Error)
        return 0;
    for (int i = 0; i < nodeNum; i++)
    {
        if (IsChild[i] != 1) //&& !strcmp(nodeList[i]->type, "Program")
        {
            // Preorder(nodeList[i], 0);
            InterCode codes = translate_Program(nodeList[i]);
            print_Codes(codes);
        }
    }

    return 0;
}

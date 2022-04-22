//#pragma once
#ifndef TNODE_H
#define TNODE_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LISTSIZE 1000
#define MAX_NUM 100

extern int yylineno;
extern char *yytext;
void yyerror(char *msg);

typedef struct Tnode
{
    int num;                 //在nodeList中的编号,终结符的num为-1
    int lineno;              //节点所在行号
    char *type;              //节点类型
    char *numtype;           // int float
    int tag;                 // 1变量 2函数 3常数 4数组 5结构体
    struct Tnode *leftchild; //左子树
    struct Tnode *next;      //当前节点的兄弟 链表
    // struct Tnode *child[10]; //子节点
    // int childnum;
    char *content; //语义值
    union
    {
        char *ID_TYPE; // ID和TYPE类型的节点的属性为字符串
        int intval;
        float fltval;
    };
} * Tree, *tnode;
// typedef struct Tnode *Tree;
// typedef struct Tnode tnode;

/*  numOfChild个tnode
    num:根节点的序号
    申请单个节点(叶节点):newTree("ID",-1,0,lineno)  属性由yytext确定
    申请两个节点:newTree("ID",num,2,tnode1,tnode2)
 */
Tree newTree(char *type, int num, int numOfChild, ...);

void Preorder(Tree rootNode, int level);

extern int nodeNum; //节点数量
/*  nodeList和IsChild的作用是在建好语法树之后寻找根节点（因为是从下向上建立），不用存储叶节点
 */
extern tnode nodeList[LISTSIZE]; //存储所有非叶节点
extern int IsChild[LISTSIZE];    // 1:nodeList[]中的对应节点不是根节点

/* ***************符号表*****************/
//由语法树建立符号表
void analysis(tnode val);

//变量符号表
typedef struct var_
{
    char *name;
    char *type;
    int instruct; //是否为结构体的域
    int structno; //结构体编号
    struct var_ *next;
} var;
extern var *varhead, *vartail;

extern char **varname[MAX_NUM];
extern int varnameno;

void newvar(int num, ...);           //建立变量符号表项
int findvar(tnode val);              //检查变量是否已定义
int getvarstr(char *name, var *pos); //由变量名获取变量信息
char *typevar(tnode val);            //查询变量类型
int checkleft(tnode val);            //赋值号左边是否合法

//函数符号表
typedef struct func_
{
    int tag; // 0 未定义 1 已定义
    char *type;
    char *name;        //函数名
    char *rtype;       //返回值类型
    int va_num;        //形参个数
    char *va_type[10]; //形参类型
    int instruct;
    int structno;
    struct func_ *next;
} func;
extern func *funchead, *functail;
extern int va_num;        //函数实参
extern char *va_type[10]; //实参类型
extern char *rtype[10];   //函数实际返回值类型
extern int rnum;
void getrtype(tnode val);                //获取实际返回值类型
void getdeftype(tnode val);              //函数形参类型
void getreatype(tnode val);              //函数实参类型
void getargs(tnode Args);                //获取函数实参
int checkfunctype(tnode ID, tnode Args); //检查形参和实参是否一致
int checkmultifuncstr(char *);

void newfunc(int num, ...);
int findfunc(tnode val);   //检查函数是否已定义
func *getfunc(tnode val);  //获取函数定义
char *typefunc(tnode val); //获取函数参数类型
int numfunc(tnode val);    //获取函数的形参个数

//数组符号表
typedef struct array_
{
    char *name;
    char *type;
    int instruct;
    int structno;
    struct array_ *next;
} array;
extern array *arrayhead, *arraytail;
void newarray(int num, ...);
int findarray(tnode val);
char *typearray(tnode val);

//结构体符号表
typedef struct fieldlist_
{
    char *name;
    struct fieldlist_ *next;
} * fieldlist;
typedef struct struct_
{
    char *name;
    char *type;
    int instruct;
    int structno;
    struct struct_ *next;
    struct fieldlist_ *fields;
} struct_;
extern struct_ *structhead, *structtail;
void newstruct(int num, ...);
int findstruct(tnode val);
int findstructstr(char *str);
struct_ *getstructstr(char *);
void addstructfield(struct_ *, tnode);
char *checkmultifield(struct_ *);
extern fieldlist fieldtail;
extern struct_ *currentstruct;
int checkstructfield(struct_ *, char *);
int findstructmember(char *structtype, char *str); //查找str是否是结构体的成员
extern int instruct;                               //当前是否是结构体的域
extern int LCnum;                                  //花括号是否闭合
extern int structno;                               //当前是第几个结构体
// extern fieldlist structfield[100]; //
// extern char **structname[100];
// extern int structnum;

/***********************中间代码生成**************************/
typedef struct _OperandStru
{
    enum
    {
        VARIABLE, //变量
        TEMPVAR,
        LABLE,
        CONSTANT,
        ADDRESS, //&
        VALUE,   //*
        FUNC
    } kind;
    union
    {
        int tempvar; //标号
        int lable;
        int value;  //常量的值
        char *name; //语义值 变量、函数名称
    } operand;
    int value; //常量的值
} OperandStru, *Operand;
typedef struct _InterCodeStru
{
    enum
    {
        _USELESS, //避免让_LABLE为0
        _LABLE,
        _FUNCTION,
        _ASSIGN,
        _ADD,
        _SUB,
        _MUL,
        _DIV,
        _GOTO,
        _IFGOTO,
        _RETURN,
        _ARG,
        _CALL,
        _PARAM,
        _READ,
        _WRITE,
        _NULL
    } kind;
    union
    {
        struct
        {
            Operand left, right;
        } assign;
        struct
        {
            Operand result, op1, op2;
        } binop; //双目运算
        struct
        {
            Operand lable, op1, op2;
            char *relop;
        } jump;
        Operand var; //单操作数
    } operands;
    struct _InterCodeStru *prev, *next;
} InterCodeStru, *InterCode;
//函数参数列表
typedef struct _ArgListStru
{
    int num;
    Operand list[10];
} ArgListStru, *ArgList;

extern InterCode CodesHead, CodesTail; //双链表首尾
//临时变量和标签
extern int tempvar[MAX_NUM];
extern int tempvarnum;         //下一个可用的临时变量下标
extern var variables[MAX_NUM]; //变量与v1 v2 的映射
extern var varinum;
extern int lables[MAX_NUM];
extern int lablesnum;
Operand new_tempvar();
Operand new_lable();

void init_tempvar_lable();
//当Exp产生INT、ID、MINUS Exp时，获取已经声明过的operand
Operand get_Operand(tnode Exp);
//查看是否已经声明过同一个常数值的操作数
Operand find_Const(int value);

void init_InterCode();
Operand new_Operand();
Operand new_Variable(char *name);
Operand new_Const(int value);
InterCode new_Code();
InterCode new_lable_Code(Operand lable);
InterCode new_goto_Code(Operand lable);
InterCode new_assign_Code(Operand left, Operand right);
void print_Code(InterCode code);
void print_Operand(Operand op);
void print_Codes(InterCode codes);   //打印一段中间代码
InterCode get_Tail(InterCode codes); //获取链表尾部
InterCode add_Codes(int num, ...);   //在链表尾部连接另一条链表

InterCode translate_Program(tnode Program);
InterCode translate_ExtDefList(tnode ExtDefList);
InterCode translate_ExtDef(tnode ExtDef);

//变量、函数声明
InterCode translate_FunDec(tnode FunDec);
InterCode translate_VarList(tnode VarList);
InterCode translate_ParamDec(tnode ParamDec);

//作用域
InterCode translate_CompSt(tnode CompSt);

InterCode translate_StmtList(tnode StmList);
InterCode translate_Stmt(tnode Stmt);

//变量声明
InterCode translate_DefList(tnode DefList);
InterCode translate_Def(tnode Def);
InterCode translate_DecList(tnode DecList);
InterCode translate_Dec(tnode);

//基本表达式
InterCode translate_Exp(tnode, Operand place);
InterCode translate_Cond(tnode, Operand lable_true, Operand lable_false);
//函数参数
InterCode translate_Args(tnode, ArgList arg_list);

char *getvarnamestr(char *); //传入变量名，返回v1 v2...，若该变量未定义则返回NULL

#endif
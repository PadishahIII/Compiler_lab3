#include "tnode.h"

int nodeNum;
tnode nodeList[LISTSIZE];
int IsChild[LISTSIZE];
int va_num;
char *va_type[10];
char *rtype[10];
int rnum;
int instruct; //当前是否是结构体的域
int LCnum;    //花括号是否闭合
int structno; //当前是第几个结构体
var *varhead, *vartail;
func *funchead, *functail;
array *arrayhead, *arraytail;
struct_ *structhead, *structtail;
// fieldlist structfield[100];
// char **structname[100];
// int structnum;
fieldlist fieldtail;
struct_ *currentstruct;

/******中间代码生成***********/
InterCode CodesHead, CodesTail; //双链表收尾
                                //临时变量和标签
int tempvar[100];
int tempvarnum; //下一个可用的临时变量下标
int lables[100];
int lablesnum;

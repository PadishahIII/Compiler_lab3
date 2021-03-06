%{
    #include "tnode.h"
    #include<unistd.h>
    #include<stdio.h>
%}

%union{
    double type_double;
    tnode type_tree;
}

%token <type_tree> INT
%token <type_tree> FLOAT
%token <type_tree> ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR 
%token <type_tree> DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%type <type_tree> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def  DecList Dec Exp Args 

%left LP RP LB RB DOT
%right NOT
%left STAR DIV
%left PLUS MINUS
%left RELOP
%left AND OR 
%right ASSIGNOP 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level definitions */
Program:ExtDefList {$$=newTree("Program",nodeNum,1,$1);nodeList[nodeNum++]=$$;int useless=@1.first_line;}
    ;
ExtDefList: {$$=newTree("ExtDefList",nodeNum,0,-1);nodeList[nodeNum++]=$$;}
    |ExtDef ExtDefList {$$=newTree("ExtDefList",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;}
    ;
ExtDef: Specifier ExtDecList SEMI {
    $$=newTree("ExtDef",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
}
    |Specifier SEMI {$$=newTree("ExtDef",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;}
    |Specifier FunDec CompSt {
        $$=newTree("ExtDef",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error8
        newfunc(1,$1);
        //Error4 检查函数是否重复定义
}
    |error SEMI
    ;
ExtDecList: VarDec {
    $$=newTree("ExtDecList",nodeNum,1,$1);nodeList[nodeNum++]=$$;
}
    |VarDec COMMA ExtDecList {$$=newTree("ExtDecList",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;}
    ;

/* Specifiers */
Specifier: TYPE {$$=newTree("Specifier",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    |StructSpecifier {
        $$=newTree("Specifier",nodeNum,1,$1);nodeList[nodeNum++]=$$;
        $$->content=$1->content;
}
    ;
StructSpecifier: STRUCT OptTag LC DefList RC {
    $$=newTree("StructSpecifier",nodeNum,5,$1,$2,$3,$4,$5);nodeList[nodeNum++]=$$;
    $$->content=$2->content;
    instruct = 0;//TODO:结构体定义完成
    //Error16
    if(findstruct($2))
        printf("Error type 16 at line %d:Duplicated name '%s'\n",yylineno,$2->content);
    else newstruct(1,$2);

    struct_*structpos = getstructstr($2->content);

    //将DefList中的域加入结构体节点
    addstructfield(structpos,$4);

    //Error15  检查结构体域是否重复定义
    char*errname = checkmultifield(structpos);
    if(errname!=NULL)
        printf("Error type 15 at line %d:Redefined field '%s'\n",yylineno,errname);
}
    |STRUCT Tag {//TODO:struct define
        $$=newTree("StructSpecifier",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;
        $$->content=$2->content;
        if($2->content!=NULL){
            //Error17 结构体未声明
            if(!findstructstr($2->content))
                printf("Error type 17 at line %d:undefined struct '%s'\n",yylineno,$2->content);
        }
}
    ;
OptTag: {$$=newTree("OptTag",nodeNum,0,-1);nodeList[nodeNum++]=$$;}
    |ID {$$=newTree("OptTag",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    ;
Tag: ID {$$=newTree("Tag",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    ;

/* Declarators */
VarDec: ID {$$=newTree("VarDec",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    |VarDec LB INT RB {
        $$=newTree("VarDec",nodeNum,4,$1,$2,$3,$4);nodeList[nodeNum++]=$$;
        $$->tag=4;
}
    ;
FunDec: ID LP VarList RP {
    $$=newTree("FunDec",nodeNum,4,$1,$2,$3,$4);nodeList[nodeNum++]=$$;
    //Error4
    $$->content = $1->content;
    if(findfunc($1))
        printf("Error type 4 at line %d:Redefined function '%s'\n",yylineno,$1->content);
    else
        newfunc(2,$1,$3);
}
    |ID LP RP {
        $$=newTree("FunDec",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error4
        $$->content = $1->content;
        if(findfunc($1))
            printf("Error type 4 at line %d:Redefined function '%s'\n",yylineno,$1->content);
        else
            newfunc(2,$1,$3);
}
    ;
VarList: ParamDec COMMA VarList {$$=newTree("VarList",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;}
    |ParamDec {$$=newTree("VarList",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    ;
ParamDec: Specifier VarDec {
    $$=newTree("ParamDec",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;
    //Error3
    if(!(instruct&&LCnum))
        if(findvar($2)||findarray($2)||findstruct($2))
            printf("Error type 3 at line %d:Redefined variable '%s'.\n",yylineno,$2->content);
    else if($2->tag==4)
        newarray(2,$1,$2);
    else 
        newvar(2,$1,$2);
}
    ;

/* Statements */
CompSt: LC DefList StmtList RC {$$=newTree("CompSt",nodeNum,4,$1,$2,$3,$4);nodeList[nodeNum++]=$$;}
    ;
StmtList: {$$=newTree("StmtList",nodeNum,0,-1);nodeList[nodeNum++]=$$;}
    |Stmt StmtList {$$=newTree("StmtList",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;}
    ;
Stmt: Exp SEMI {$$=newTree("Stmt",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;}
    |CompSt {$$=newTree("Stmt",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    |RETURN Exp SEMI {
        $$=newTree("Stmt",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error8
        getrtype($2);
}
    |IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$=newTree("Stmt",nodeNum,5,$1,$2,$3,$4,$5);nodeList[nodeNum++]=$$;}
    |IF LP Exp RP Stmt ELSE Stmt {$$=newTree("Stmt",nodeNum,7,$1,$2,$3,$4,$5,$6,$7);nodeList[nodeNum++]=$$;}
    |WHILE LP Exp RP Stmt {$$=newTree("Stmt",nodeNum,5,$1,$2,$3,$4,$5);nodeList[nodeNum++]=$$;}
    |error SEMI
    ;

/* Local Definitions*/
DefList: {$$=newTree("DecList",nodeNum,0,-1);nodeList[nodeNum++]=$$;}
    |Def DefList {$$=newTree("DecList",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;}
    ;
Def: Specifier DecList SEMI {
    $$=newTree("Def",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
    //Error3
    if(!(instruct&&LCnum))
        if(findvar($2)||findarray($2)||findstructstr($2->content)||findstructstr(typevar($2)))//
            printf("Error type 3 at line %d:Redefined variable '%s'.\n",yylineno,$2->content);
    else if($2->tag==4)
        newarray(2,$1,$2);
    else 
        getVaris($2);
        newvar(2,$1,$2);
}
    |error SEMI
    ;
DecList:Dec {$$=newTree("DecList",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    |Dec COMMA DecList {$$=newTree("DecList",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;}
    ;
Dec: VarDec {$$=newTree("Dec",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    |VarDec ASSIGNOP Exp {$$=newTree("Dec",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;}
    ;

/* Expressions */
Exp: Exp ASSIGNOP Exp {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error5
        //if($1->numtype==NULL||$3->numtype==NULL){
            //printf("Error0\n");
            //return;
        //}
        //if(strcmp($1->numtype,$3->numtype)){
            //printf("Error type 5 at line %d:type mismatch(%s %s %s).\n",yylineno,$1->numtype,$2->content,$3->numtype);
        //}
        ////Error6
        //if(!checkleft($1)){
            //printf("Error type 6 at line %d:The left-hand side of the assignment must be a variable.\n",yylineno);
        //}
    }
    |Exp AND Exp {$$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;}
    |Exp OR Exp {$$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;}
    |Exp RELOP Exp {$$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;}
    |Exp PLUS Exp {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;
        //Error7
        //if(strcmp($1->numtype,$3->numtype))//TODO:
         //   printf("Error type 7 at line %d:Type mismatched for operands.\n",yylineno);
    }
    |Exp MINUS Exp {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;
        //Error7
        //if(strcmp($1->numtype,$3->numtype))
        //    printf("Error type 7 at line %d:Type mismatched for operands.\n",yylineno);
    }
    |Exp STAR Exp {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;
        //Error7
        //if(strcmp($1->numtype,$3->numtype))
        //    printf("Error type 7 at line %d:Type mismatched for operands.\n",yylineno);
    }
    |Exp DIV Exp {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $1->numtype;
        //Error7
        //if(strcmp($1->numtype,$3->numtype))
        //    printf("Error type 7 at line %d:Type mismatched for operands.\n",yylineno);
    }
    |LP Exp RP {$$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;$$->numtype = $2->numtype;}
    |MINUS Exp {$$=newTree("Exp",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;$$->numtype = $2->numtype;}
    |NOT Exp {$$=newTree("Exp",nodeNum,2,$1,$2);nodeList[nodeNum++]=$$;$$->numtype = $2->numtype;}
    |ID LP Args RP {
        $$=newTree("Exp",nodeNum,4,$1,$2,$3,$4);nodeList[nodeNum++]=$$;
        //Error11
        if(!findfunc($1)&&(findvar($1)||findarray($1)))
            printf("Error type 11 at line %d:'%s' is not a function.\n",yylineno,$1->content);
        //Error2
        else if(!findfunc($1))
            printf("Error type 2 at line %d:undefined function '%s'.\n",yylineno,$1->content);
        //Error9
        else if(checkrtype($1,$3)){
            printf("Error type 9 at line %d:Function %s(%s) is not applicable for args (",yylineno,$1->content,typefunc($1));
            for(int i = 0;i<va_num-1;i++)
                printf("%s,",va_type[i]);
            printf("%s)\n",va_type[va_num-1]);
        }
       // $$->numtype = getfunc($1->content)->rtype;
    }
    |ID LP RP {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error11
        if(!findfunc($1)&&(findvar($1)||findarray($1)))
            printf("Error type 11 at line %d:'%s' is not a function.\n",yylineno,$1->content);
        //Error2
        else if(!findfunc($1))
            printf("Error type 2 at line %d:undefined function '%s'.\n",yylineno,$1->content);
        //TODO:获取函数返回值类型
        func* temp = getfunc($1);
        $$->numtype=temp->rtype;
    }
    |Exp LB Exp RB {
        $$=newTree("Exp",nodeNum,4,$1,$2,$3,$4);nodeList[nodeNum++]=$$;
        //Error10
        //if(!findarray($1))
           // printf("Error type 10 at line %d: '%s' is not an array\n",yylineno,$1->content);
        //Error12
        if(strcmp($3->numtype,"int"))
            printf("Error type 12 at line %d: '%f' is not an integer\n",yylineno,$3->fltval);
}
    |Exp DOT ID {
        $$=newTree("Exp",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;
        //Error1:变量使用时未定义
        if(!findvar($1)&&!findarray($1))
            printf("Error type 1 at line %d: undefined variable '%s'\n",yylineno,$1->content);
        //Error13
        if(findarray($1)||!findstructstr(typevar($1))){
            printf("Error type 13 at line %d: Illegal use of dot\n",yylineno);
            return;
        }
        //Error14
        struct_*structpos = getstructstr(typevar($1));
        if(!checkstructfield(structpos,$3->content))
            printf("Error type 14 at line %d: Non-existent field '%s'\n",yylineno,$3->content);
}
    |ID {$$=newTree("Exp",nodeNum,1,$1);nodeList[nodeNum++]=$$;
        //Error1:变量使用时未定义
        if(!findvar($1)&&!findarray($1))
            printf("Error type 1 at line %d: undefined variable '%s'\n",yylineno,$1->content);
        else
            $$->numtype=typevar($1);
        }
    |INT {
        $$=newTree("Exp",nodeNum,1,$1);nodeList[nodeNum++]=$$;
        $$->numtype="int";
        $$->intval = $1->intval;//TODO:
}
    |FLOAT {
        $$=newTree("Exp",nodeNum,1,$1);nodeList[nodeNum++]=$$;
        $$->numtype="float";
        $$->fltval = $1->fltval;
}
    ;
Args: Exp COMMA Args {$$=newTree("Args",nodeNum,3,$1,$2,$3);nodeList[nodeNum++]=$$;}
    |Exp {$$=newTree("Args",nodeNum,1,$1);nodeList[nodeNum++]=$$;}
    ;

%%
#include "lex.yy.c"




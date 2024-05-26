#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "syntax.h"
#include "lexer.h"
#include "utils.h"
#include "domain.h"
#include "types.h"


Token *consumedTk, *crtTk;

Symbol *crtStruct, *crtFunc;

void addVar(Token *tkName,Type *t)
{
    Symbol *s;
    if(crtStruct){
        if(findSymbol(&crtStruct->members,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&crtStruct->members,tkName->text,CLS_VAR);
    }
    else if(crtFunc){
        s=findSymbol(&symbols,tkName->text);
        if(s&&s->depth==crtDepth)
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&symbols,tkName->text,CLS_VAR);
        s->mem=MEM_LOCAL;
    }
    else{
        if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&symbols,tkName->text,CLS_VAR);   
        s->mem=MEM_GLOBAL;
    }
    s->type=*t;
}

int consume(int code){
    if(crtTk->code==code){
        consumedTk=crtTk;
        crtTk=crtTk->next;
        return 1;
    }
    return 0;
}

int expr(RetVal *rv), arrayDecl(Type *ret);

// typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(Type *ret){
    if(consume(INT)){
        ret->typeBase=TB_INT;
        return 1;
    }
    if(consume(DOUBLE)){
        ret->typeBase=TB_DOUBLE;
        return 1;
    }
    if(consume(CHAR)){
        ret->typeBase=TB_CHAR;
        return 1;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            Symbol      *s=findSymbol(&symbols,tkName->text);
            if(s==NULL)tkerr(crtTk,"undefined symbol: %s",tkName->text);
            if(s->cls!=CLS_STRUCT)tkerr(crtTk,"%s is not a struct",tkName->text);
            ret->typeBase=TB_STRUCT;
            ret->s=s;
            return 1;
        }
    }
    return 0;
}

// typeName: typeBase arrayDecl? 
int typeName(Type *ret){
    if(!typeBase(ret)) return 0;
    if(arrayDecl(ret)){
    }
    else{
        ret->nElements=-1;
    }
    return 1;
}

/*
exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
           | CT_INT
           | CT_REAL 
           | CT_CHAR 
           | CT_STRING 
           | LPAR expr RPAR ;
*/
int exprPrimary(RetVal *rv){
    Token *tkName;
    RetVal arg;
    if(consume(ID)){
        tkName = consumedTk;
        Symbol *s=findSymbol(&symbols,tkName->text);
        if(!s)tkerr(crtTk,"undefined symbol %s",tkName->text);
        rv->type=s->type;
        rv->isCtVal=0;
        rv->isLVal=1;

        if(consume(LPAR)){
            Symbol **crtDefArg=s->args.begin;
            if(s->cls!=CLS_FUNC&&s->cls!=CLS_EXTFUNC)
                tkerr(crtTk,"call of the non-function %s",tkName->text);

            if(!consume(RPAR)){
                if(expr(&arg)){
                    if(crtDefArg==s->args.end)tkerr(crtTk,"too many arguments in call");
                    cast(&(*crtDefArg)->type,&arg.type);
                    crtDefArg++;

                    while(1){
                        if(consume(COMMA)){
                            if(!expr(&arg)) tkerr(crtTk, "expected expression after ,");
                            else{
                                if(crtDefArg==s->args.end)
                                    tkerr(crtTk,"too many arguments in call");
                                cast(&(*crtDefArg)->type,&arg.type);
                                crtDefArg++;
                            }
                        }
                        else break;
                    }
                }
                if(!consume(RPAR)) tkerr(crtTk, "expected )");
                else{
                    if(crtDefArg!=s->args.end)
                        tkerr(crtTk,"too few arguments in call");
                    rv->type=s->type;
                    rv->isCtVal=rv->isLVal=0;
                }
            }
            if(s->cls==CLS_FUNC||s->cls==CLS_EXTFUNC)
                tkerr(crtTk,"missing call for function %s",tkName->text);
        }
    }
    else if(consume(CT_INT)){
        Token *tki = consumedTk;
        rv->type=createType(TB_INT,-1);
        rv->ctVal.i=tki->i;
        rv->isCtVal=1;
        rv->isLVal=0;
    }
    else if(consume(CT_REAL)){
        Token *tkr = consumedTk;
        rv->type=createType(TB_DOUBLE,-1);
        rv->ctVal.d=tkr->r;
        rv->isCtVal=1;
        rv->isLVal=0;
    }
    else if(consume(CT_CHAR)){
        Token *tkc = consumedTk;
        rv->type=createType(TB_CHAR,-1);
        rv->ctVal.i=tkc->i;
        rv->isCtVal=1;
        rv->isLVal=0;
    }
    else if(consume(CT_STRING)){
        Token *tks = consumedTk;
        rv->type=createType(TB_CHAR,0);
        rv->ctVal.str=tks->text;
        rv->isCtVal=1;
        rv->isLVal=0;
    }
    else if(consume(LPAR)){
        if(!expr(rv)) tkerr(crtTk, "Expected expression after (");
        else if (!consume(RPAR)) tkerr(crtTk, "Expected ) after expression");
    }
    else return 0;
    return 1;
}

/*
exprPostfix: exprPostfix LBRACKET expr RBRACKET
           | exprPostfix DOT ID 
           | exprPrimary ;
*/
int exprPostfix1(RetVal *rv){
    RetVal rve;
    Token *tkName;
    if(consume(LBRACKET)){
        if(!expr(&rve)) tkerr(crtTk, "Expected expression after [");
        if(rv->type.nElements<0)tkerr(crtTk,"only an array can be indexed");
            Type typeInt=createType(TB_INT,-1);
            cast(&typeInt,&rve.type);
            rv->type=rv->type;
            rv->type.nElements=-1;
            rv->isLVal=1;
            rv->isCtVal=0;
        if(!consume(RBRACKET)) tkerr(crtTk, "Expected ] after expression");
    }
    else if(consume(DOT)){
        if(!consume(ID)) tkerr(crtTk, "Expected ID after .");
        tkName = consumedTk;
        Symbol      *sStruct=rv->type.s;
        Symbol      *sMember=findSymbol(&sStruct->members,tkName->text);
        if(!sMember)
            tkerr(crtTk,"struct %s does not have a member %s",sStruct->name,tkName->text);
        rv->type=sMember->type;
        rv->isLVal=1;
        rv->isCtVal=0;
    }
    return 1;
}

int exprPostfix(RetVal *rv){
    if(!exprPrimary(rv)) return 0;
    if(!exprPostfix1(rv)) return 0;
    return 1;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(RetVal *rv){
    if(consume(SUB)){
        if(!exprUnary(rv)) tkerr(crtTk, "expected unary expression");
        else{
            if(rv->type.nElements>=0)tkerr(crtTk,"unary '-' cannot be applied to an array");
            if(rv->type.typeBase==TB_STRUCT)
                tkerr(crtTk,"unary '-' cannot be applied to a struct");
            rv->isCtVal=rv->isLVal=0;
        }
    }
    else if(consume(NOT)){
        if(!exprUnary(rv)) tkerr(crtTk, "expected unary expression");
        else{
            if(rv->type.typeBase==TB_STRUCT)tkerr(crtTk,"'!' cannot be applied to a struct");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
    }
    else if(exprPostfix(rv)){
    }
    else return 0;
    return 1;
}

// exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(RetVal *rv){
    Type t;
    RetVal rve;
    if(consume(LPAR)){ 
        if(!typeName(&t)) return 0;
        if(!consume(RPAR)) return 0;
        if(!exprCast(&rve)) return 0;
        else{
            cast(&t,&rve.type);
            rv->type=t;
            rv->isCtVal=rv->isLVal=0;
        }
    }
    else if(exprUnary(rv)){
    }
    else return 0;
    return 1;
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
int exprMul1(RetVal *rv){
    RetVal rve;
    if(consume(MUL)){
        if(!exprCast(&rve)) tkerr(crtTk, "expected expression after *");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be multiplied or divided");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprMul1(rv)){
        }
    }
    else if(consume(DIV)){
        if(!exprCast(&rve)) tkerr(crtTk, "expected expression after /");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be multiplied or divided");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprMul1(rv)){
        }
    }
    return 1;
}

int exprMul(RetVal *rv){
    if(!exprCast(rv)) return 0;
    if(!exprMul1(rv)) return 0;
    return 1;
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
int exprAdd1(RetVal *rv){
    RetVal rve;
    if(consume(ADD)){
        if(!exprMul(&rve)) tkerr(crtTk, "expected expression after +");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be added or subtracted");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;        
        }
        if(exprAdd1(rv)){
        }
    }
    else if(consume(SUB)){
        if(!exprMul(&rve)) tkerr(crtTk, "expected expression after -");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be added or subtracted");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;        
        }
        if(exprAdd1(rv)){
        }
    }
    return 1;
}

int exprAdd(RetVal *rv){
    if(!exprMul(rv)) return 0;
    if(!exprAdd1(rv)) return 0;
    return 1;
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
int exprRel1(RetVal *rv){
    RetVal rve;
    if(consume(LESS)){
        if(!exprAdd(&rve)) tkerr(crtTk, "expected expression after <");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprRel1(rv)){
        }
    }
    else if(consume(LESSEQ)){
        if(!exprAdd(&rve)) tkerr(crtTk, "expected expression after <=");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprRel1(rv)){
        }
    }
    else if(consume(GREATER)){
        if(!exprAdd(&rve)) tkerr(crtTk, "expected expression after >");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprRel1(rv)){
        }
    }
    else if(consume(GREATEREQ)){
        if(!exprAdd(&rve)) tkerr(crtTk, "expected expression after >=");
        else{
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprRel1(rv)){
        }
    }
    return 1;
}

int exprRel(RetVal *rv){
    if(!exprAdd(rv)) return 0;
    if(!exprRel1(rv)) return 0;
    return 1;
}


//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel ;
int exprEq1(RetVal *rv){
    RetVal rve;
    if(consume(EQUAL)){
        if(!exprRel(&rve)) tkerr(crtTk, "expected expression after ==");
        else{
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprEq1(rv)){
        }
    }
    else if(consume(NOTEQ)){
        if(!exprRel(rv)) tkerr(crtTk, "expected expression after !=");
        else{
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprEq1(rv)){
        }
    }
    return 1;
}

int exprEq(RetVal *rv){
    if(!exprRel(rv)) return 0;
    if(!exprEq1(rv)) return 0;
    return 1;
}

// exprAnd: exprAnd AND exprEq | exprEq ;
int exprAnd1(RetVal *rv){
    RetVal rve;
    if(consume(AND)){
        if(!exprEq(&rve)) tkerr(crtTk, "expected expression after &&");
        else{
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprAnd1(rv)){
        }       
    }
    return 1;
}

int exprAnd(RetVal *rv){
    if(!exprEq(rv)) return 0;
    if(!exprAnd1(rv)) return 0;
    return 1;
}

// exprOr: exprOr OR exprAnd | exprAnd ;
int exprOr1(RetVal *rv){
    RetVal rve;
    if(consume(OR)){
        if(!exprAnd(&rve)) tkerr(crtTk, "expected expression after ||");
        else{
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
        }
        if(exprOr1(rv)){
        }
    }
    return 1;
}

int exprOr(RetVal *rv){
    if(!exprAnd(rv)) return 0;
    if(!exprOr1(rv)) return 0; 
    return 1;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign(RetVal *rv){
    Token *startTk = crtTk;
    RetVal rve;
    if(exprUnary(rv)) {
        if(consume(ASSIGN)){ 
            if(!exprAssign(&rve)) tkerr(crtTk, "missing assign expression");

            if(!rv->isLVal)tkerr(crtTk,"cannot assign to a non-lval");
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"the arrays cannot be assigned");
            cast(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;
        }
        //else tkerr(crtTk, "missing = in assign expression");
        else {
            crtTk = startTk;
        }
    }
    if(exprOr(rv)){
    }
    else return 0;
    return 1;
}

// expr: exprAssign ;
int expr(RetVal *rv){
    if(!exprAssign(rv)) return 0;
    return 1;
}

// arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(Type *ret){
    if(!consume(LBRACKET))return 0;
    RetVal rv;
    if(expr(&rv)){
        //ret->nElements=0; 

        if(!rv.isCtVal)tkerr(crtTk,"the array size is not a constant");
        if(rv.type.typeBase!=TB_INT)tkerr(crtTk,"the array size is not an integer");
        ret->nElements=rv.ctVal.i;
    }
    else{
        ret->nElements=0; 
    }
    if(!consume(RBRACKET))tkerr(crtTk,"missing ] or syntax error"); 
    return 1;
}

/*
stm: stmCompound 
           | IF LPAR expr RPAR stm ( ELSE stm )?
           | WHILE LPAR expr RPAR stm
           | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
           | BREAK SEMICOLON
           | RETURN expr? SEMICOLON
           | expr? SEMICOLON ;
*/
int stm(), declVar(), stmCompound();

int ruleIf(){
    RetVal rv;
    if(!consume(IF)) return 0;
    if(!consume(LPAR)) tkerr(crtTk,"expected ( after if");
    if(!expr(&rv)) tkerr(crtTk,"expected expression in if condition");

    if(rv.type.typeBase==TB_STRUCT)
        tkerr(crtTk,"a structure cannot be logically tested");
                
    if(!consume(RPAR)) tkerr(crtTk,"expected ) after expression");
    if(!stm()) tkerr(crtTk,"expected statement after if condition");
    if(consume(ELSE)){
        if(!stm()) tkerr(crtTk,"expected statement after else");
    }
    return 1;
}

int ruleWhile()
{
    RetVal rv;
    if(!consume(WHILE))return 0;
    if(!consume(LPAR))tkerr(crtTk,"missing ( after while");
    if(!expr(&rv))tkerr(crtTk,"invalid expression after (");

    if(rv.type.typeBase==TB_STRUCT)
        tkerr(crtTk,"a structure cannot be logically tested");

    if(!consume(RPAR))tkerr(crtTk,"missing )");
    if(!stm())tkerr(crtTk,"missing while statement");
    return 1;
}

int ruleFor(){
    RetVal rv1, rv2, rv3;
    if(!consume(FOR)) return 0;
    if(!consume(LPAR))tkerr(crtTk,"missing ( after for");
    if(expr(&rv1)){
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after expr");
    if(expr(&rv2)){
        if(rv2.type.typeBase==TB_STRUCT)
            tkerr(crtTk,"a structure cannot be logically tested");
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after expr");
    if(expr(&rv3)){
    }
    if(!consume(RPAR))tkerr(crtTk,"missing )");
    if(!stm())tkerr(crtTk,"missing for statement");
    return 1;
}

int ruleBreak(){
    if(!consume(BREAK)) return 0;
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after break");
    return 1;
}

int ruleReturn(){
    RetVal rv;
    if(!consume(RETURN)) return 0;
    if(expr(&rv)){
        if(crtFunc->type.typeBase==TB_VOID)
            tkerr(crtTk,"a void function cannot return a value");
        cast(&crtFunc->type,&rv.type);
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after break");
    return 1;
}

int ruleExpr(){
    RetVal rv;
    if(expr(&rv)){
    }
    if(!consume(SEMICOLON)) return 0;
    return 1;
}

int stm(){
    if(stmCompound()){
    }
    else if(ruleIf()){
    }
    else if(ruleWhile()){
    }
    else if(ruleWhile()){
    }
    else if(ruleFor()){
    }
    else if(ruleBreak()){
    }
    else if(ruleReturn()){
    }
    else if(ruleExpr()){
    }
    else return 0;
    return 1;
}

// stmCompound: LACC ( declVar | stm )* RACC ; 

int stmCompound(){
        Symbol *start=symbols.end[-1];
    if(!consume(LACC))return 0;
    crtDepth++;
    while(1){
        if(declVar()){
        }
        else if(stm()){
        }
        else break;
    }
    if(!consume(RACC))tkerr(crtTk,"missing } or syntax error");
    
    crtDepth--;
    deleteSymbolsAfter(&symbols,start);
    return 1;
}

// funcArg: typeBase ID arrayDecl? ;

int funcArg(){
    Type t;
    Token *tkName;
    if(!typeBase(&t)) return 0;
    if(!consume(ID)) tkerr(crtTk,"missing argument ID or syntax error");
    tkName = consumedTk;
    if(arrayDecl(&t)){
    }
    else{
        t.nElements=-1; 
    }
    Symbol  *s=addSymbol(&symbols,tkName->text,CLS_VAR);
    s->mem=MEM_ARG;
    s->type=t;
    s=addSymbol(&crtFunc->args,tkName->text,CLS_VAR);
    s->mem=MEM_ARG;
    s->type=t;
    return 1;
}

/* declFunc: ( typeBase MUL? | VOID ) ID
                    LPAR ( funcArg ( COMMA funcArg )* )? RPAR 
                        stmCompound ;
*/

int declFunc(){
    Token *startTk = crtTk;
    Token *tkName;
    Type t;
    if(typeBase(&t)){
        if(consume(MUL)){
            t.nElements=0;
        }
        else{
            t.nElements=-1;
        }
    }
    else if(consume(VOID)){
        t.typeBase=TB_VOID;
    }
    else return 0;
    if(!consume(ID)) 
        return 0;
    tkName = consumedTk;
    if(!consume(LPAR)){
        crtTk = startTk;
        return 0;
    }
    if(findSymbol(&symbols,tkName->text))
        tkerr(crtTk,"symbol redefinition: %s",tkName->text);
    crtFunc=addSymbol(&symbols,tkName->text,CLS_FUNC);
    initSymbols(&crtFunc->args);
    crtFunc->type=t;
    crtDepth++;

    if(funcArg()){
    }
    while(1){
        if(consume(COMMA)){
            if(!funcArg()) tkerr(crtTk,"expected argument after , ");
        }
        else break;
    }
    if(!consume(RPAR)) tkerr(crtTk,"expected ) after function declaration");
    crtDepth--;
    if(!stmCompound()) tkerr(crtTk,"expected statement in function");
    
    deleteSymbolsAfter(&symbols,crtFunc);
    crtFunc=NULL;

    return 1;
}

// declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(){
    Token *tkName;
    Type t;
    if(!typeBase(&t))return 0;
    if(!consume(ID)) tkerr(crtTk, "expected variable name");
    tkName = consumedTk;
    if(arrayDecl(&t)){
    }
    else{
        t.nElements=-1;
    }
    addVar(tkName,&t);
    while(1){
        if(consume(COMMA)){
            if(consume(ID)){
                tkName = consumedTk;
                if(arrayDecl(&t)){
                }
                else{
                    t.nElements=-1;
                }
                addVar(tkName,&t);
            }
        }
        else break;
    }
    if(!consume(SEMICOLON))tkerr(crtTk,"missing ; or syntax error");
    return 1;
}

// declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct(){
    Token *startTk = crtTk;
    if(!consume(STRUCT)) return 0;
    if(!consume(ID)) tkerr(crtTk,"expected struct ID");
    Token *tkName = consumedTk;
    if(!consume(LACC)) {crtTk = startTk; return 0;};

    if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        crtStruct=addSymbol(&symbols,tkName->text,CLS_STRUCT);
        initSymbols(&crtStruct->members);

    while(1){
        if(declVar()){
        }
        else break;
    }


    if(!consume(RACC)) tkerr(crtTk,"expected }");
    if(!consume(SEMICOLON))tkerr(crtTk,"missing ; or syntax error");
    crtStruct=NULL;
    return 1;
}

// unit: ( declStruct | declFunc | declVar )* END ;
int unit(){
    while(1){
        if(declStruct()){
        }
        else if(declFunc()){
        }
        else if(declVar()){
        }
        else break;
    }
    if(!consume(END))tkerr(crtTk, "syntax error");
    return 1;
}

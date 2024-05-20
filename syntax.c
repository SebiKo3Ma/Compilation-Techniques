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

Token *consumedTk, *crtTk;

Symbol *crtStruct, *crtFunc;
Token *tkName;
int crtDepth;
Type t, *ret;

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

int expr(), arrayDecl();

// typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(){
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
int typeName(){
    if(!typeBase()) return 0;
    if(arrayDecl()){
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
int exprPrimary(){
    if(consume(ID)){
        if(consume(LPAR)){
            if(!consume(RPAR)){
                if(expr()){
                    while(1){
                        if(consume(COMMA)){
                            if(!expr()) tkerr(crtTk, "expected expression after ,");
                        }
                        else break;
                    }
                }
                if(!consume(RPAR)) tkerr(crtTk, "expected )");
            }
        }
    }
    else if(consume(CT_INT)){
    }
    else if(consume(CT_REAL)){
    }
    else if(consume(CT_CHAR)){
    }
    else if(consume(CT_STRING)){
    }
    else if(consume(LPAR)){
        if(!expr()) tkerr(crtTk, "Expected expression after (");
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
int exprPostfix1(){
    if(consume(LBRACKET)){
        if(!expr()) tkerr(crtTk, "Expected expression after [");
        if(!consume(RBRACKET)) tkerr(crtTk, "Expected ] after expression");
    }
    else if(consume(DOT)){
        if(!consume(ID)) tkerr(crtTk, "Expected ID after .");
    }
    return 1;
}

int exprPostfix(){
    if(!exprPrimary()) return 0;
    if(!exprPostfix1()) return 0;
    return 1;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(){
    if(consume(SUB) || consume(NOT)){
        if(!exprUnary()) tkerr(crtTk, "expected unary expression");
    }
    else if(exprPostfix()){
    }
    else return 0;
    return 1;
}

// exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(){
    if(consume(LPAR)){ 
        if(!typeName()) return 0;
        if(!consume(RPAR)) return 0;
        if(!exprCast()) return 0;
    }
    else if(exprUnary()){
    }
    else return 0;
    return 1;
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
int exprMul1(){
    if(consume(MUL)){
        if(!exprCast()) tkerr(crtTk, "expected expression after *");
        if(exprMul1()){
        }
    }
    else if(consume(DIV)){
        if(!exprCast()) tkerr(crtTk, "expected expression after /");
        if(exprMul1()){
        }
    }
    return 1;
}

int exprMul(){
    if(!exprCast()) return 0;
    if(!exprMul1()) return 0;
    return 1;
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
int exprAdd1(){
    if(consume(ADD)){
        if(!exprMul()) tkerr(crtTk, "expected expression after +");
        if(exprAdd1()){
        }
    }
    else if(consume(SUB)){
        if(!exprMul()) tkerr(crtTk, "expected expression after -");
        if(exprAdd1()){
        }
    }
    return 1;
}

int exprAdd(){
    if(!exprMul()) return 0;
    if(!exprAdd1()) return 0;
    return 1;
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
int exprRel1(){
    if(consume(LESS)){
        if(!exprAdd()) tkerr(crtTk, "expected expression after <");
            if(exprRel1()){
        }
    }
    else if(consume(LESSEQ)){
        if(!exprAdd()) tkerr(crtTk, "expected expression after <=");
            if(exprRel1()){
        }
    }
    else if(consume(GREATER)){
        if(!exprAdd()) tkerr(crtTk, "expected expression after >");
            if(exprRel1()){
        }
    }
    else if(consume(GREATEREQ)){
        if(!exprAdd()) tkerr(crtTk, "expected expression after >=");
            if(exprRel1()){
        }
    }
    return 1;
}

int exprRel(){
    if(!exprAdd()) return 0;
    if(!exprRel1()) return 0;
    return 1;
}


//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel ;
int exprEq1(){
    if(consume(EQUAL)){
        if(!exprRel()) tkerr(crtTk, "expected expression after ==");
            if(exprEq1()){
        }
    }
    else if(consume(NOTEQ)){
        if(!exprRel()) tkerr(crtTk, "expected expression after !=");
        if(exprEq1()){
        }
    }
    return 1;
}

int exprEq(){
    if(!exprRel()) return 0;
    if(!exprEq1()) return 0;
    return 1;
}

// exprAnd: exprAnd AND exprEq | exprEq ;
int exprAnd1(){
    if(consume(AND)){
        if(!exprEq()) tkerr(crtTk, "expected expression after &&");
        if(exprAnd1()){
        }       
    }
    return 1;
}

int exprAnd(){
    if(!exprEq()) return 0;
    if(!exprAnd1()) return 0;
    return 1;
}

// exprOr: exprOr OR exprAnd | exprAnd ;
int exprOr1(){
    if(consume(OR)){
        if(!exprAnd()) tkerr(crtTk, "expected expression after ||");
        if(exprOr1()){
        }
    }
    return 1;
}

int exprOr(){
    if(!exprAnd()) return 0;
    if(!exprOr1()) return 0; 
    return 1;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign(){
    Token *startTk = crtTk;
    if(exprUnary()) {
        if(consume(ASSIGN)){ 
            if(!exprAssign()) tkerr(crtTk, "missing assign expression");
        }
        //else tkerr(crtTk, "missing = in assign expression");
        else {
            crtTk = startTk;
        }
    }
    if(exprOr()){
    }
    else return 0;
    return 1;
}

// expr: exprAssign ;
int expr(){
    if(!exprAssign()) return 0;
    return 1;
}

// arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(){
    if(!consume(LBRACKET))return 0;
    if(expr()){
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
    if(!consume(IF)) return 0;
    if(!consume(LPAR)) tkerr(crtTk,"expected ( after if");
    if(!expr()) tkerr(crtTk,"expected expression in if condition");
    if(!consume(RPAR)) tkerr(crtTk,"expected ) after expression");
    if(!stm()) tkerr(crtTk,"expected statement after if condition");
    if(consume(ELSE)){
        if(!stm()) tkerr(crtTk,"expected statement after else");
    }
    return 1;
}

int ruleWhile()
{
    if(!consume(WHILE))return 0;
    if(!consume(LPAR))tkerr(crtTk,"missing ( after while");
    if(!expr())tkerr(crtTk,"invalid expression after (");
    if(!consume(RPAR))tkerr(crtTk,"missing )");
    if(!stm())tkerr(crtTk,"missing while statement");
    return 1;
}

int ruleFor(){
    if(!consume(FOR)) return 0;
    if(!consume(LPAR))tkerr(crtTk,"missing ( after for");
    if(expr()){
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after expr");
    if(expr()){
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after expr");
    if(expr()){
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
    if(!consume(RETURN)) return 0;
    if(expr()){
    }
    if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after break");
    return 1;
}

int ruleExpr(){
    if(expr()){
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
    if(!typeBase()) return 0;
    if(!consume(ID)) tkerr(crtTk,"missing argument ID or syntax error");
    if(arrayDecl()){
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

int check;

int declFunc(){
    check = 0;

    if(typeBase()){
        if(consume(MUL)){
            check = -1;
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
    else check++;
    if(!consume(LPAR)) return 0;

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
    check = 0;
    if(!stmCompound()) tkerr(crtTk,"expected statement in function");
    
    deleteSymbolsAfter(&symbols,crtFunc);
    crtFunc=NULL;

    return 1;
}

// declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(){
    if(!typeBase() && !check)return 0;
    if(!consume(ID) && !check) tkerr(crtTk, "expected variable name");
    check = 0;
    if(arrayDecl()){
    }
    else{
        t.nElements=-1;
    }
    addVar(tkName,&t);
    while(1){
        if(consume(COMMA)){
            if(consume(ID)){
                if(arrayDecl()){
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
    if(!consume(LACC)) {crtTk = startTk; return 0;};

    if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        crtStruct=addSymbol(&symbols,tkName->text,CLS_STRUCT);
        initSymbols(&crtStruct->members);

    while(1){
        check = 0;
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
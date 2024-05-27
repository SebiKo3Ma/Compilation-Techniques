#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "lexer.h"

char *hex = "0123456789abcdefABCDEF";
char *esc = "abfnrtv'?\"\\0";
char *esc2 = "\a\b\f\n\r\t\v\'\?\"\\\0";



Token *tokens, *lastToken;
int line = 1;

char *pCrtCh;

//temporary
const char *enum_names[43] = {
    [ID] = "ID",
    [END] = "END",
    [BREAK] = "BREAK",
    [CHAR] = "CHAR",
    [DOUBLE] = "DOUBLE",
    [ELSE] = "ELSE",
    [FOR] = "FOR",
    [IF] = "IF",
    [INT] = "INT",
    [RETURN] = "RETURN",
    [STRUCT] = "STRUCT",
    [VOID] = "VOID",
    [WHILE] = "WHILE",
    [CT_INT] = "CT_INT",
    [CT_REAL] = "CT_REAL",
    [CT_CHAR] = "CT_CHAR",
    [CT_STRING] = "CT_STRING",
    [COMMA] = "COMMA",
    [SEMICOLON] = "SEMICOLON",
    [LPAR] = "LPAR",
    [RPAR] = "RPAR",
    [LBRACKET] = "LBRACKET",
    [RBRACKET] = "RBRACKET",
    [LACC] = "LACC",
    [RACC] = "RACC",
    [ADD] = "ADD",
    [SUB] = "SUB",
    [MUL] = "MUL",
    [DIV] = "DIV",
    [DOT] = "DOT",
    [AND] = "AND",
    [OR] = "OR",
    [NOT] = "NOT",
    [NOTEQ] = "NOTEQ",
    [ASSIGN] = "ASSIGN",
    [EQUAL] = "EQUAL",
    [LESS] = "LESS",
    [LESSEQ] = "LESSEQ",
    [GREATER] = "GREATER",
    [GREATEREQ] = "GREATEREQ"
};


Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk,Token);
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
        }else{
    tokens=tk;
    }
    lastToken=tk;
    return tk;
}


char *createString(const char *pStartCh, const char *pEndCh){

    // Calculate the length of the string between the two pointers
    size_t length = pEndCh - pStartCh;

    // Allocate memory for the new string
    char *new_string = (char *)malloc(length + 1); // Plus one for the null terminator

    // Check if memory allocation was successful
    if (new_string == NULL) {
        printf("Memory allocation failed\n");
        exit(0);
    }

    // Copy the substring between the pointers
    strncpy(new_string, pStartCh, length);
    new_string[length] = '\0'; // Null-terminate the string

    return new_string;
}

char *escapeString(char *string) {
    int len = strlen(string);
    char *newString = (char *)malloc(len + 1); // Allocate memory for the new string
    if (!newString) {
        return NULL; // Handle allocation failure
    }
    int i = 0, j = 0;

    while (i < len) {
        if (string[i] == '\\') {
            i++; // Skip the backslash
            if (i >= len) break; // Avoid out-of-bounds access if backslash is the last character
        }
        newString[j++] = string[i++];
    }
    newString[j] = '\0'; // Null-terminate the new string

    return newString;
}

int getNextToken()
{
    int state=0,nCh;
    char ch;
    const char *pStartCh;
    Token *tk;
    int escape = 0;

    while(1){ // infinite loop
        ch=*pCrtCh;
        switch(state){
            case 0: // transitions test for state 0
                if(isalpha(ch)||ch=='_'){
                    pStartCh=pCrtCh; // memorizes the beginning of the ID
                    pCrtCh++; // consume the character
                    state=1; // set the new state
                }
                else if(ch==' '||ch=='\r'||ch=='\t'){
                    pCrtCh++; // consume the character and remains in state 0
                }
                else if(ch=='\n'){ // handled separately in order to update the current line
                    line++;
                    pCrtCh++;
                }
                else if(ch==0){ // the end of the input string
                   // if(isdigit(ch)) printf("True\n");
                    addTk(END);
                    return END;
                }
                else if(isdigit(ch) && ch != '0'){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 3;
                }
                else if(isdigit(ch) && ch == '0'){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 5;
                }
                else if(ch == '\''){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 15;
                }
                else if(ch == '\"'){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 19;
                }
                else if(ch==','){
                    pCrtCh++;
                    state=23;
                }
                else if(ch==';'){
                    pCrtCh++;
                    state=24;
                }
                else if(ch=='('){
                    pCrtCh++;
                    state=25;
                }else if (ch == ')') {
                    pCrtCh++;
                    state = 26;
                }
                else if (ch == '[') {
                    pCrtCh++;
                    state = 27;
                }
                else if (ch == ']') {
                    pCrtCh++;
                    state = 28;
                }
                else if (ch == '{') {
                    pCrtCh++;
                    state = 29;
                }
                else if (ch == '}') {
                    pCrtCh++;
                    state = 30;
                }
                else if (ch == '+') {
                    pCrtCh++;
                    state = 31;
                }
                else if (ch == '-') {
                    pCrtCh++;
                    state = 32;
                }
                else if (ch == '*') {
                    pCrtCh++;
                    state = 33;
                }
                else if (ch == '.') {
                    pCrtCh++;
                    state = 35;
                }
                else if (ch == '&') {
                    pCrtCh++;
                    state = 36;
                }
                else if (ch == '|') {
                    pCrtCh++;
                    state = 37;
                }
                else if (ch == '!') {
                    pCrtCh++;
                    state = 40;
                }
                else if (ch == '=') {
                    pCrtCh++;
                    state = 41;
                }
                else if (ch == '<') {
                    pCrtCh++;
                    state = 42;
                }
                else if (ch == '>') {
                    pCrtCh++;
                    state = 43;
                }
                else if (ch == '/'){
                    pCrtCh++;
                    state = 52;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 1:
                if(isalnum(ch)||ch=='_')pCrtCh++;
                    else state=2;

                break;
            case 2:
                nCh=pCrtCh-pStartCh; // the id length
                // keywords tests
                if(nCh==5&&!memcmp(pStartCh,"break",5))tk=addTk(BREAK);
                else if(nCh==4&&!memcmp(pStartCh,"char",4))tk=addTk(CHAR);
                else if(nCh==6&&!memcmp(pStartCh,"double",6))tk=addTk(DOUBLE);
                else if(nCh==4&&!memcmp(pStartCh,"else",4))tk=addTk(ELSE);
                else if(nCh==3&&!memcmp(pStartCh,"for",3))tk=addTk(FOR);
                else if(nCh==2&&!memcmp(pStartCh,"if",2))tk=addTk(IF);
                else if(nCh==3&&!memcmp(pStartCh,"int",3))tk=addTk(INT);
                else if(nCh==6&&!memcmp(pStartCh,"return",6))tk=addTk(RETURN);
                else if(nCh==6&&!memcmp(pStartCh,"struct",6))tk=addTk(STRUCT);
                else if(nCh==4&&!memcmp(pStartCh,"void",4))tk=addTk(VOID);
                else if(nCh==5&&!memcmp(pStartCh,"while",5))tk=addTk(WHILE);
                // … all keywords …
                else{ // if no keyword, then it is an ID
                    tk=addTk(ID);
                    tk->text=createString(pStartCh,pCrtCh);
                }
                return tk->code;
            case 3:
                if(isdigit(ch)) pCrtCh++;
                else if(ch == '.'){
                    pCrtCh++;
                    state = 12;
                }
                else if (ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 9;
                }
                else state = 4;
                break;
            case 4:
                tk = addTk(CT_INT);
                tk -> i = strtol(pStartCh, &pCrtCh, 0);
                return tk->code;
            case 5:
                if(isdigit(ch) && ch != '8' && ch !='9') pCrtCh++;
                else if (ch == '8' || ch == '9'){
                    pCrtCh++;
                    state = 8;
                }
                else if (ch == 'x'){
                    pCrtCh++;
                    state = 6;
                }       
                else if (ch == '.'){
                    pCrtCh++;
                    state = 12;
                }
                else if (ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 9;
                }
                else{
                    state = 4;
                }
                break;
            case 6:
                //printf("%c", ch);
                if(strchr(hex, ch)){
                    pCrtCh++;
                    state = 7;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 7:
                //printf("%c", ch);
                if(strchr(hex, ch)) pCrtCh++;
                else{
                    state = 4;
                }
                break;
            case 8:
                if(isdigit(ch)) pCrtCh++;
                else if(ch == '.'){
                    pCrtCh++;
                    state = 12;
                }
                else if (ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 9;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 9:
                if(ch == '+' || ch == '-') pCrtCh++;
                state = 10;
                break;
            case 10:
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 11;
                }
                else tkerr(addTk(END),"invalid character. Expected digit. 2 ");
                break;
            case 11:
                if(isdigit(ch)){
                    pCrtCh++;
                }
                else state = 14;
                break;
            case 12:
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 13;
                }
                else tkerr(addTk(END),"invalid character. Expected digit. 1");
                break;
            case 13:
                if(isdigit(ch)){
                    pCrtCh++;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 9;
                }
                else state = 14;
                break;
            case 14:
                tk = addTk(CT_REAL);
                tk -> r = atof(createString(pStartCh, pCrtCh));
                return tk->code;
            case 15:
                pStartCh = pCrtCh;
                if(ch == '\\'){
                    pCrtCh++;
                    state = 17;
                }
                else if(ch != '\''){
                    pCrtCh++;
                    state = 16;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 16:
                if(ch == '\''){
                    pCrtCh++;
                    state = 18;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 17:
                pStartCh = pCrtCh;
                escape = 1;
                if(strchr(esc, ch)){
                    pCrtCh++;
                    state = 16;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 18:
                tk = addTk(CT_CHAR);
                if(escape == 0)
                    tk -> i = pStartCh[0];
                else{
                    tk->i = esc2[strchr(esc, pStartCh[0]) - esc];
                    escape = 0;
                }
                return tk->code;
            case 19:
                pStartCh = pCrtCh;
                if(ch == '\\'){
                    pCrtCh++;
                    state = 21;
                }
                else if(ch != '\"'){
                    if(ch == '\n'){
                        line++;
                    }
                    pCrtCh++;
                    state = 20;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 20:
                if(ch == '\"'){
                    state = 22;
                }
                else if(ch != '\\'){
                    if(ch == '\n'){
                        line++;
                    }
                    pCrtCh++;
                }
                else if(ch == '\\'){
                    pCrtCh++;
                    state = 21;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 21:
                if(strchr(esc, ch)){
                    pCrtCh++;
                    state = 20;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 22:
                tk = addTk(CT_STRING);

                char *string = createString(pStartCh, pCrtCh);
                char *escaped = escapeString(string);

                printf("Original: %s\n", string);
                printf("Escaped: %s\n", escaped);
                tk -> text = escaped;

                pCrtCh++;
                return tk->code;
            case 23:
                addTk(COMMA);
                return COMMA;
            case 24:
                addTk(SEMICOLON);
                return SEMICOLON;
            case 25:
                addTk(LPAR);
                return LPAR;
            case 26:
                addTk(RPAR);
                return RPAR;
            case 27:
                addTk(LBRACKET);
                return LBRACKET;
            case 28:
                addTk(RBRACKET);
                return RBRACKET;
            case 29:
                addTk(LACC);
                return LACC;
            case 30:
                addTk(RACC);
                return RACC;
            case 31:
                addTk(ADD);
                return ADD;
            case 32:
                addTk(SUB);
                return SUB;
            case 33:
                addTk(MUL);
                return MUL;
            case 34:
                addTk(DIV);
                return DIV;
            case 35:
                addTk(DOT);
                return DOT;
            case 36:
                if(ch=='&'){
                    pCrtCh++;
                    state=38;
                }
                else tkerr(addTk(END),"invalid character: &");
                break;
            case 37:
                if(ch=='|'){
                    pCrtCh++;
                    state=39;
                }
                else tkerr(addTk(END),"invalid character: |");
                break;
            case 38:
                addTk(AND);
                return AND;
            case 39:
                addTk(OR);
                return OR;
            case 40:
                if(ch=='='){
                    pCrtCh++;
                    state=45;
                }
                else state=44;
                break;
            case 41:
                if(ch=='='){
                    pCrtCh++;
                    state=47;
                }
                else state=46;
                break;
            case 42:
                if(ch=='='){
                    pCrtCh++;
                    state=49;
                }
                else state=48;
                break;
            case 43:
                if(ch=='='){
                    pCrtCh++;
                    state=51;
                }
                else state=50;
                break;
            case 44:
                addTk(NOT);
                return NOT;
            case 45:
                addTk(NOTEQ);
                return NOTEQ;
            case 46:
                addTk(ASSIGN);
                return ASSIGN;
            case 47:
                addTk(EQUAL);
                return EQUAL;
            case 48:
                addTk(LESS);
                return LESS;
            case 49:
                addTk(LESSEQ);
                return LESSEQ;
            case 50:
                addTk(GREATER);
                return GREATER;
            case 51:
                addTk(GREATEREQ);
                return GREATEREQ;
            case 52:
                if(ch == '/'){
                    pCrtCh++;
                    state = 53;
                    line++;
                }
                else if(ch == '*'){
                    pCrtCh++;
                    state = 54;
                }
                else{
                    state = 34;
                };
                break;
            case 53:
                if(!(ch == '\n' || ch == '\r' || ch == '\0')){
                    pCrtCh++;
                }
                else{
                    pCrtCh++;
                    state = 0;
                }
                break;
            case 54:
                if(!(ch == '*')){
                    if(ch == '\n'){
                        line++;
                    }
                    pCrtCh++;
                }
                else if(ch == '*'){
                    pCrtCh++;
                    state = 55;
                }
                break;
            case 55:
                if(ch == '*'){
                    pCrtCh++;
                }
                else if(ch == '/'){
                    pCrtCh++;
                    state = 0;
                }
                else{
                    pCrtCh++;
                    state = 54;
                    if(ch == '\n'){
                        line++;
                    }
                }
                break;
        }
    }
}

void tempTestPrint(){
    Token *temp;
    temp = tokens;
    while(temp!=NULL){
    printf("%s ", enum_names[temp->code]);
    if(temp->code == 0 || temp->code == 16)
        printf("%s ", temp->text);
    else if(temp->code == 13)
        printf("%ld ", temp->i);
    else if(temp->code == 15)
        printf("%c %ld ", (int) temp->i, temp->i);
    else if(temp->code == 14)
        printf("%f", temp->r);
    printf("%d\n", temp->line);
    temp = temp->next;
    }
}
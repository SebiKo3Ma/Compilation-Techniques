#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum{ID, END, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, 
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, SUB, MUL, DIV, DOT,
    AND, OR, NOT, NOTEQ, ASSIGN, EQUAL, LESS, LESSEQ, GREATER, GREATEREQ}; // tokens codes

char *hex = "0123456789abcdefABCDEF";
char *esc = "abfnrtv'?\"\\0";
char *esc2 = "\a\b\f\n\r\t\v\'\?\"\\\0";

typedef struct _Token{
    int code; // code (name)
    union{
        char *text; // used for ID, CT_STRING (dynamically allocated)
        long int i; // used for CT_INT, CT_CHAR
        double r; // used for CT_REAL
    };
    int line; // the input file line
    struct _Token *next; // link to the next token
}Token;

Token *tokens, *lastToken;
int line = 1;

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

void err(const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

char *pCrtCh;

void readFile(char *infile){
    long length;
    FILE *f = fopen(infile, "rb");
    if (f){
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        pCrtCh = malloc (length);
        if (pCrtCh)
        {
            fread (pCrtCh, 1, length, f);
        }
        fclose (f);
    }
}

char *createString(char *pStartCh, char *pEndCh){

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
                    if(isdigit(ch)) printf("True\n");
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
                printf("%c", ch);
                if(strchr(hex, ch)){
                    pCrtCh++;
                    state = 7;
                }
                else tkerr(addTk(END),"invalid character ");
                break;
            case 7:
                printf("%c", ch);
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
                else if(ch != '\''){
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
                tk -> text = createString(pStartCh, pCrtCh);
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
                    pCrtCh++;
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
        printf("%c %ld ", temp->i, temp->i);
    else if(temp->code == 14)
        printf("%f", temp->r);
    printf("%d\n", temp->line);
    temp = temp->next;
    }
}

Token *consumedTk, *crtTk;

int consume(int code){
    if(crtTk->code==code){
        consumedTk=crtTk;
        crtTk=crtTk->next;
        return 1;
    }
    return 0;
}


// typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(){
    if(consume(INT)){
        return 1;
    }
    if(consume(DOUBLE)){
        return 1;
    }
    if(consume(CHAR)){
        return 1;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            return 1;
        }
    }
    return 0;
}

int expr(){
    if(!consume(CT_INT)) return 0;
    return 1;
}

// arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(){
    if(!consume(LBRACKET))return 0;
    expr();
    if(!consume(RBRACKET))tkerr(crtTk,"missing ] or syntax error"); 
    return 1;
}

// stmCompound: LACC ( declVar | stm )* RACC ; 
/*
int stmCompound(){
    if(!consume(LACC))return 0;
    while(1){
        if(declVar()){
        }
        else if(stm()){
        }
        else break;
    }
    if(!consume(RACC))tkerr(crtTk,"missing } or syntax error");
    return 1;
}*/

/* declFunc: ( typeBase MUL? | VOID ) ID
                    LPAR ( funcArg ( COMMA funcArg )* )? RPAR 
                        stmCompound ;
*/

// funcArg: typeBase ID arrayDecl? ;

int funcArg(){
    if(!typeBase()) return 0;
    if(!consume(ID)) tkerr(crtTk,"missing argument ID or syntax error");
    if(arrayDecl()){
    }
    return 1;
}

int check;

int declFunc(){
    check = 0;

    if(typeBase()){
        if(consume(MUL)){
            check = -1;
        }
    }
    else if(consume(VOID)){
    }
    else return 0;
    if(!consume(ID)) 
        return 0;
    else check++;
    if(!consume(LPAR)) return 0;
    if(funcArg()){
    }
    while(1){
        if(consume(COMMA)){
            if(!funcArg()) tkerr(crtTk,"expected argument after , ");
        }
        else break;
    }
    if(!consume(RPAR)) tkerr(crtTk,"expected ) after function declaration");
    return 1;
}

// declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(){
    if(!typeBase() && !check)return 0;
    if(!consume(ID) && !check)return 0;
    if(arrayDecl()){}
    while(1){
        if(consume(COMMA)){
            if(consume(ID)){
                if(arrayDecl()){
                }
            }
        }
        else break;
    }
    if(!consume(SEMICOLON))tkerr(crtTk,"missing ; or syntax error");
    return 1;
}

// declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct(){
    if(!consume(STRUCT)) return 0;
    if(!consume(ID)) tkerr(crtTk,"expected struct ID");
    if(!consume(LACC)) tkerr(crtTk,"expected { after ID");
    while(1){
        check = 0;
        if(declVar()){
        }
        else break;
    }
    if(!consume(RACC)) tkerr(crtTk,"expected }");
    if(!consume(SEMICOLON))tkerr(crtTk,"missing ; or syntax error");
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

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Invalid usage!");
        exit(1);
    }
    readFile(argv[1]);
    while(getNextToken() != END){} // lexical analysis
    tempTestPrint(); // test print token list
    crtTk = tokens;
    unit();
}

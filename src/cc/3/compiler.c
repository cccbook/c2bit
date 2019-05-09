#include <assert.h>
#include "compiler.h"

#define _ NULL

char* EXP();
char* E();
char* CALL(char *fname);
void STMT();
void IF();
void BLOCK();
void FUNCTION();

Token tnow, tnext;

int tempIdx = 1, labelIdx = 1;

char* nextTemp() {
  return stPrint("t%d", tempIdx++);
}

char *nextLabel() {
  return stPrint("L%d", labelIdx++);
}

int isNext(char *set) {
  char eset[SMAX], etoken[SMAX+2];
  sprintf(eset, " %s ", set);
  sprintf(etoken, " %s ", tnext.str);
  return (tnext.type != End && strstr(eset, etoken) != NULL);
}

int isNextType(TokenType type) {
  return (tnext.type == type);
}

int isEnd() {
  return tnext.type == End;
}

char *next() {
  tnow = tnext;
  tnext = lexScan();
  debug("token = %-10s type = %-10s\n", tnext.str, tokenTypeName[tnext.type]);
  return tnow.str;
}

char *skip(char *set) {
  if (isNext(set)) {
    return next();
  } else {
    error("skip(%s) got %s fail!\n", set, next());
  }
}

char *skipType(TokenType type) {
  if (isNextType(type)) {
    return next();
  } else {
    error("skipType(%s) got %s fail!\n", tokenTypeName[type], tokenTypeName[tnext.type]);
  }
}

#define ARGMAX 100

char *RT = "t0"; // 傳回值永遠放在 RT 暫存器(變數)裏

char* CALL(char *fname) {
  char* args[ARGMAX];
  int top = 0;
  skip("(");
  if (!isNext(")")) {
    args[top++] = E();
    while (isNext(",")) {
      skip(",");
      args[top++] = E();
      assert(top < ARGMAX);
    }
  }
  skip(")");
  for (int i=0; i<top; i++) {
    irEmit(IrArg, args[i], _, _, _);
  }
  irEmit(IrCall, fname, _, _, _);
  return RT;
}

// F = (E) | Number | Id | CALL
char* F() {
  char* f;
  if (isNext("(")) { // '(' E ')'
    skip("("); // (
    f = E();
    skip(")"); // )
  } else {
    char *item = next();
    if (isNext("(")) { // id (...) ==> CALL
      f = CALL(item);
    } else { // Number | Id
      return item;
    }
  }
  return f;
}

// E = F (op E)*
char* E() {
  char* p1 = F();
  while (isNext("+ - * / & | < > = <= >= == != && ||")) {
    char *op = skipType(Op);
    char* p2 = E();
    char* p0 = nextTemp();
    irEmit(IrOp2, p0, p1, p2, op);
    p1 = p0;
  }
  return p1;
}

// EXP = E
char* EXP() {
  tempIdx = 1; // 讓 temp 重新開始，才不會 temp 太多！
  return E();
}

// return E;
char* RETURN() {
  skip("return");
  char* e = EXP();
  irEmit(IrReturn, e, _, _, _);
  skip(";");
  return e;
}

// ASSIGN(id) =  '=' E
void ASSIGN(char *id) {
  skip("=");
  char* e = EXP();
  irEmit(IrAssign, id, e, _, _);
}

// WHILE = while (E) STMT
void WHILE() {
  char* whileBegin = nextLabel();
  char* whileEnd = nextLabel();
  irEmit(IrLabel, whileBegin, _, _, _);
  skip("while");
  skip("(");
  char *e = EXP();
  irEmit(IrIfNotGoto, e, whileEnd, _, _);
  skip(")");
  STMT();
  irEmit(IrGoto, whileBegin, _, _, _);
  irEmit(IrLabel, whileEnd, _, _, _);
}

// if (E) STMT (else STMT)?
void IF() {
  char* elseBegin = nextLabel();
  char* ifEnd = nextLabel();
  skip("if");
  skip("(");
  char* e = EXP();
  irEmit(IrIfNotGoto, e, elseBegin, _, _);
  skip(")");
  STMT();
  irEmit(IrGoto, ifEnd, _, _, _);
  irEmit(IrLabel, elseBegin, _, _, _);
  if (isNext("else")) {
    skip("else");
    STMT();
  }
  irEmit(IrLabel, ifEnd, _, _, _);
}

// LABEL = id ':'
void LABEL(char *id) {
  skip(":");
  irEmit(IrLabel, id, _, _, _);
}

// GOTO = goto id ;
void GOTO() {
  skip("goto");
  char *id = skipType(Id);
  skip(";");
  irEmit(IrGoto, id, _, _, _);
}

// ASM = asm( string ) ;
void ASM() {
  skip("asm");
  skip("(");
  char *asmCode = skipType(String);
  skip(")");
  skip(";");
  irEmit(IrAsm, asmCode, _, _, _);
}

// STMT = WHILE | IF  | BLOCK | ASM | GOTO | LABEL | ASSIGN ; | CALL ; | RETURN
void STMT() {
  if (isNext("while"))
    WHILE();
  else if (isNext("if"))
    IF();
  else if (isNext("{"))
    BLOCK();
  else if (isNext("asm"))
    ASM();
  else if (isNext("goto"))
    GOTO();
  else if (isNext("return"))
    RETURN();
  else if (isNext("def"))
    FUNCTION();
  else {
    char *id = skipType(Id);
    if (isNext("=")) {
      ASSIGN(id);
      skip(";");
    } else if (isNext(":")) {
      LABEL(id);
    } else if (isNext("(")) {
      CALL(id);
      skip(";");
    }
  }
}

// STMTS = STMT*
void STMTS() {
  while (!isEnd() && !isNext("}")) {
    STMT();
  }
}

// BLOCK = { STMT* }
void BLOCK() {
  skip("{");
  STMTS();
  skip("}");
}

// PARAMS = id {,id}*
void PARAMS() {
  while (isNextType(Id)) {
    if (isNext(")")) break;
    char *id = skipType(Id);
    irEmit(IrParam, id, _, _, _);
    if (isNext(",")) skip(",");
  }
}

// FUNCTION = def id (PARAMS) BLOCK
void FUNCTION() {
  skip("def");
  char *id = skipType(Id);
  irEmit(IrFunction, id, _, _, _);
  skip("(");
  PARAMS();
  skip(")");
  BLOCK();
  irEmit(IrFend, _, _, _, _);
}

// PROG = STMT*
void PROG() {
  STMTS();
}

void parse(char *code) {
  debug("============ parse =============\n");
  lexInit(code);
  next();
  PROG();
}

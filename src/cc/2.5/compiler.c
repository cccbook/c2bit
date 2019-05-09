#include <assert.h>
#include "compiler.h"

int EXP();
int  E();
void STMT();
void IF();
void BLOCK();
void FUNCTION();

Token tnow, tnext;

int tempIdx = 1, labelIdx = 1;

#define nextTemp() (tempIdx++)
#define nextLabel() (labelIdx++)

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

int CALL(char *fname) {
  int args[ARGMAX];
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
    irEmit(IrArg, args[i], 0, 0, NULL);
  }
  irEmit(IrCall, 0, 0, 0, fname);
  return 0; // 傳回值永遠放在 t0
}

// F = (E) | Number | Id | CALL
int F() {
  int f;
  if (isNext("(")) { // '(' E ')'
    skip("("); // (
    f = E();
    skip(")"); // )
  } else {
    char *item = next();
    if (isNext("(")) { // id (...) ==> CALL
      f = CALL(item);
    } else { // Number | Id
      f = nextTemp();
      irEmit(IrAssignTs, f, 0, 0, item);
    }
  }
  return f;
}

// E = F (op E)*
int E() {
  int i1 = F();
  while (isNext("+ - * / & | < > = <= >= == != && ||")) {
    char *op = skipType(Op);
    int i2 = E();
    int i = nextTemp();
    irEmit(IrOp2, i, i1, i2, op);
    i1 = i;
  }
  return i1;
}

// EXP = E
int EXP() {
  tempIdx = 1; // 讓 temp 重新開始，才不會 temp 太多！
  return E();
}

// return E;
int RETURN() {
  skip("return");
  int e = EXP();
  irEmit(IrReturn, e, 0, 0, NULL);
  skip(";");
  return e;
}

// ASSIGN(id) =  '=' E
void ASSIGN(char *id) {
  skip("=");
  int e = EXP();
  irEmit(IrAssignSt, e, 0, 0, id);
}

// WHILE = while (E) STMT
void WHILE() {
  int whileBegin = nextLabel();
  int whileEnd = nextLabel();
  irEmit(IrLabel, whileBegin, 0, 0, NULL);
  skip("while");
  skip("(");
  int e = EXP();
  irEmit(IrIfNotGoto, e, whileEnd, 0, NULL);
  skip(")");
  STMT();
  irEmit(IrGoto, whileBegin, 0, 0, NULL);
  irEmit(IrLabel, whileEnd, 0, 0, NULL);
}

// if (E) STMT (else STMT)?
void IF() {
  int elseBegin = nextLabel();
  int ifEnd = nextLabel();
  skip("if");
  skip("(");
  int e = EXP();
  irEmit(IrIfNotGoto, e, elseBegin, 0, NULL);
  skip(")");
  STMT();
  irEmit(IrGoto, ifEnd, 0, 0, NULL);
  irEmit(IrLabel, elseBegin, 0, 0, NULL);
  if (isNext("else")) {
    skip("else");
    STMT();
  }
  irEmit(IrLabel, ifEnd, 0, 0, NULL);
}

// LABEL = id ':'
void LABEL(char *id) {
  skip(":");
  irEmit(IrLabel, 0, 0, 0, id);
}

// GOTO = goto id ;
void GOTO() {
  skip("goto");
  char *id = skipType(Id);
  skip(";");
  irEmit(IrGoto, 0, 0, 0, id);
}

// ASM = asm( string ) ;
void ASM() {
  skip("asm");
  skip("(");
  char *asmCode = skipType(String);
  skip(")");
  skip(";");
  irEmit(IrAsm, 0, 0, 0, asmCode);
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
  int i = 0;
  while (isNextType(Id)) {
    if (isNext(")")) break;
    char *id = skipType(Id);
    irEmit(IrParam, i++, 0, 0, id);
    if (isNext(",")) skip(",");
  }
}

// FUNCTION = def id (PARAMS) BLOCK
void FUNCTION() {
  skip("def");
  char *id = skipType(Id);
  irEmit(IrFunction, 0, 0, 0, id);
  skip("(");
  PARAMS();
  skip(")");
  BLOCK();
  irEmit(IrFend, 0, 0, 0, NULL);
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

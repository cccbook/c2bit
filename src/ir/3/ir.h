#ifndef __IR_H__
#define __IR_H__

#include <stdio.h>
#include "../../lib/strTable.h"
#include "../../lib/map.h"
#include "../../lib/util.h"

// IR Virtual Machine
#define trace printf
#define VAR_MAX 10000
#define IR_MAX  10000

typedef enum { IrAssign, IrOp2, IrLabel, IrGoto, IrIfGoto, IrIfNotGoto, IrAsm, IrCall, IrArg, IrReturn, IrParam, IrLocal, IrFunction, IrFend } IrOp;

typedef struct {
  IrOp op;
  char *p0, *p1, *p2, *str;
} IR;

extern IR ir[];
extern int irTop;
// extern int L[];

extern void irPrint(IR *p);
extern void irEmit(IrOp op, char* p, char* p1, char* p2, char *str);
extern void irWrite(FILE *fp, IR *p);
extern void irSave(char *fname);
extern void irDump();
extern void ir2m();

#endif

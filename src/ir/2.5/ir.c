#include "ir.h"

IR ir[IR_MAX];
int irTop = 0;
int Label[VAR_MAX]; // label => address

void irNew(IR p) {
  ir[irTop++] = p;
}

void irEmit(IrOp op, int p0, int p1, int p2, char *str) {
  irNew((IR) {.op=op, .p0=p0, .p1=p1, .p2=p2, .str=str});
}

void irWrite(FILE *fp, IR *p) {
  char *str = p->str;
  IrOp op = p->op;
  int p0=p->p0, p1=p->p1, p2=p->p2;
  // printf("p->str=%s\n", p->str);
  switch (op) {
    case IrAssignSt: fprintf(fp, "%s = t%d", str, p0); break;
    case IrAssignTs: fprintf(fp, "t%d = %s", p0, str); break;
    case IrLabel: if (str!=NULL) fprintf(fp, "(%s)", str); else fprintf(fp, "(L%d)", p0); break;
    case IrGoto: if (str!=NULL) fprintf(fp, "goto %s", str); else fprintf(fp, "goto L%d", p0); break;
    case IrIfGoto: fprintf(fp, "if t%d goto L%d", p0, p1); break;
    case IrIfNotGoto: fprintf(fp, "ifnot t%d goto L%d", p0, p1); break;
    case IrAsm: fprintf(fp, "asm %s", str); break;
    case IrOp2: fprintf(fp, "t%d = t%d %s t%d", p0, p1, str, p2); break;
    case IrCall: fprintf(fp, "call %s", str); break;
    case IrArg:  fprintf(fp, "arg t%d", p0); break;
    case IrParam: fprintf(fp, "param %s", str); break;
    case IrFunction: fprintf(fp, "function %s", str); break;
    case IrFend: fprintf(fp, "fend"); break;
    case IrReturn: fprintf(fp, "return t%d", p0); break;
    default: error("ir.type %d not found!", op);
  }
  fprintf(fp, "\n");
}

void irSave(char *fname) {
  FILE *irF = fopen(fname, "w");
  for (int i=0; i<irTop; i++) {
    irWrite(irF, &ir[i]);
  }
  fclose(irF);
}

void irDump() {
  printf("=======irDump()==========\n");
  for (int i=0; i<irTop; i++) {
    printf("%02d: ", i);
    irWrite(stdout, &ir[i]);
  }
}

void irPass2() {
  debug("==========irPass2()============\n");
  for (int i=0; i<irTop; i++) {
    IrOp op = ir[i].op;
    if (op == IrLabel) {
      int label = ir[i].p0;
      assert(label != 0);
      Label[label] = i;
      debug("L%d=%d\n", label, Label[label]);
    }
  }
}

/*
void irEmitAssignTs(int t, char *s) {
  irNew((IR) {.type=IrAssignTs, .t=t, .str=s});
}

void irEmitAssignSt(char *s, int t) {
  irNew((IR) {.type=IrAssignSt, .t=t, .str=s});
}

void irEmitOp2(int t, int t1, char *op, int t2) {
  irNew((IR) {.type=IrOp2, .str=op, .t=t, .t1=t1, .t2=t2});
}

void irEmitLabel(int label) {
  irNew((IR) {.type=IrLabel, .label=label});
}

void irEmitLabelStr(char *id) {
  irNew((IR) {.type=IrLabelStr, .str=id});
}

void irEmitGoto(int label) {
  irNew((IR) {.type=IrGoto, .label=label});
}

void irEmitGotoStr(char *id) {
  irNew((IR) {.type=IrGotoStr, .str=id});
}

void irEmitIfGoto(int t, int label) {
  irNew((IR) {.type=IrIfGoto, .t=t, .label=label});
}

void irEmitIfNotGoto(int t, int label) {
  irNew((IR) {.type=IrIfNotGoto, .t=t, .label=label});
}

void irEmitAsm(char *asmCode, char *args[]) {
  char *code = stAdd(asmCode);
  irNew((IR) {.type=IrAsm, .str=code });
}

void irEmitArg(int i, int t) {
  irNew((IR) {.type=IrArg, .t=t, .t1=i});
}

void irEmitCall(char *fname, int t, int label) {
  irNew((IR) {.type=IrCall, .str=fname, .t=t, .label=label});
}

void irEmitReturn(int t) {
  irNew((IR) {.type=IrReturn, .t=t});
}

void irEmitParam(char *name) {
  irNew((IR) {.type=IrParam, .str=name });
}

void irEmitFunction(char *name) {
  irNew((IR) {.type=IrFunction, .str=name });
}

void irEmitFend() {
  irNew((IR) {.type=IrFend });
}
*/
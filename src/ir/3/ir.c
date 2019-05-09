#include "ir.h"

IR ir[IR_MAX];
int irTop = 0;
// int Label[VAR_MAX]; // label => address

void irNew(IR p) {
  ir[irTop++] = p;
}

void irEmit(IrOp op, char* p0, char* p1, char* p2, char* str) {
  irNew((IR) {.op=op, .p0=p0, .p1=p1, .p2=p2, .str=str});
}

void irWrite(FILE *fp, IR *p) {
  char *p0=p->p0, *p1=p->p1, *p2=p->p2, *str=p->str;
  switch (p->op) {
    case IrAssign: fprintf(fp, "%s = %s", p0, p1); break;
    case IrLabel: fprintf(fp, "(%s)", p0); break;
    case IrGoto: fprintf(fp, "goto %s", p0); break;
    case IrIfGoto: fprintf(fp, "if %s goto %s", p0, p1); break;
    case IrIfNotGoto: fprintf(fp, "ifnot %s goto %s", p0, p1); break;
    case IrAsm: fprintf(fp, "asm %s", p0); break;
    case IrOp2: fprintf(fp, "%s = %s %s %s", p0, p1, str, p2); break;
    case IrCall: fprintf(fp, "call %s", p0); break;
    case IrArg:  fprintf(fp, "arg %s", p0); break;
    case IrParam: fprintf(fp, "param %s", p0); break;
    case IrFunction: fprintf(fp, "function %s", p0); break;
    case IrFend: fprintf(fp, "fend"); break;
    case IrReturn: fprintf(fp, "return %s", p0); break;
    default: error("ir.type %d not found!", p->op);
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

char *fname = NULL;

void ir2macro(FILE *fp, IR *p) {
  char* p0=p->p0, *p1=p->p1, *p2=p->p2, *str = p->str;
  char asmText[SMAX], tempText[SMAX], digit;
  int slen;
  if (isDebug) irWrite(stdout, p);
  switch (p->op) {
    case IrAssign:
      digit = isdigit(*p1) ? 'c' : ' ';
      fprintf(fp, ".set%c %s = %s", digit, p0, p1); // t=s
      break;
    case IrLabel: // (label)
      fprintf(fp, "(%s)", p0);
      break;
    case IrGoto: // goto label
      fprintf(fp, ".goto %s", p0);
      break;
    case IrIfGoto: // if t goto label
      fprintf(fp, ".if %s goto %s", p0, p1);
      break;
    case IrIfNotGoto: // ifnot t goto label
      fprintf(fp, ".ifnot %s goto %s", p0, p1);
      break;
    case IrAsm: // asm "...."
      slen = strlen(p0)-2;
      strncpy(tempText, &p0[1], slen);
      tempText[slen] = '\0';
      cstr2text(tempText, asmText);
      fprintf(fp, "%s", asmText);
      break;
    case IrOp2: // t = t1 op t2
      fprintf(fp, ".op   %s = %s %s %s", p0, p1, str, p2);
      break;
    case IrCall:
      fprintf(fp, ".call %s", p0); // t = call fname
      break;
    case IrArg:
      fprintf(fp, ".arg  %s %s", p0, p1); // arg i t
      break;
    case IrParam:
      fprintf(fp, ".param %s", p0); // param var
      break;
    case IrFunction:
      fname = str;
      fprintf(fp, ".function %s", fname);
      break;
    case IrFend:
      fprintf(fp, ".fend %s", fname);
      fname = NULL;
      break;
    case IrReturn:
      fprintf(fp, ".ret %s", p0);
      break;
    default: 
      error("ir.type %d not found!", p->op);
  }
  fprintf(fp, "\n");
}

void ir2m(char *mFile) {
  FILE *mF = fopen(mFile, "w");
  for (int i=0; i<irTop; i++) {
    ir2macro(mF, &ir[i]);
  }
  fclose(mF);
}

/*


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

*/
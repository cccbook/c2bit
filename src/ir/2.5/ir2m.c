#include "ir2m.h"

char *fname = NULL;

void ir2macro(FILE *fp, IR *p) {
  int p0=p->p0, p1=p->p1, p2=p->p2, slen;
  char *str = p->str, asmText[SMAX], tempText[SMAX], digit;
  if (isDebug) irWrite(stdout, p);
  switch (p->op) {
    case IrAssignSt:
      fprintf(fp, ".set  %s = t%d", str, p0); // s=t
      break;
    case IrAssignTs:
      digit = isdigit(*str) ? 'c' : ' ';
      fprintf(fp, ".set%c t%d = %s", digit, p0, str); // t=s
      break;
    case IrLabel: // (label)
      if (str == NULL) fprintf(fp, "(L%d)", p0); else fprintf(fp, "(%s)", str);
      break;
    case IrGoto: // goto label
      if (str == NULL) fprintf(fp, ".goto L%d", p0); else fprintf(fp, ".goto %s", str);
      break;
    case IrIfGoto: // if t goto label
      fprintf(fp, ".if t%d goto L%d", p0, p1);
      break;
    case IrIfNotGoto: // ifnot t goto label
      fprintf(fp, ".ifnot t%d goto L%d", p0, p1);
      break;
    case IrAsm: // asm "...."
      slen = strlen(str)-2;
      strncpy(tempText, &str[1], slen);
      tempText[slen] = '\0';
      cstr2text(tempText, asmText);
      fprintf(fp, "%s", asmText);
      break;
    case IrOp2: // t = t1 op t2
      fprintf(fp, ".op   t%d = t%d %s t%d", p0, p1, str, p2);
      break;
    case IrCall:
      fprintf(fp, ".call %s", str); // t = call fname
      break;
    case IrArg:
      fprintf(fp, ".arg  %d t%d", p0, p1); // arg i t
      break;
    case IrParam:
      fprintf(fp, ".param %d %s", p0, str); // param i var
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
      fprintf(fp, ".ret t%d %s", p0, fname);
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
#include "ir.h"

Pair varList[VAR_MAX];
Map varMap;
int v[VAR_MAX];
int t[VAR_MAX] = { 0 };
/*
int T[VAR_MAX]; // 臨時變數 T[i]
int G[VAR_MAX]; // 全域變數 G[i], 用在 map(name) => i
int Stack[VAR_MAX]; // 堆疊 <== 參數 P[i] 和區域變數 L[i] 放這裡
*/

int *varLookup(char *name) {
  int h = mapFindIdx(&varMap, name);
  return &v[h];
}

int *varAdd(char *name) {
  return mapAdd(&varMap, name, &t[0])->value;
}

int irOp2(int a, char *op, int b) {
  if (eq(op, "+")) return a + b;
  if (eq(op, "-")) return a - b;
  if (eq(op, "*")) return a * b;
  if (eq(op, "/")) return a / b;
  if (eq(op, "%%")) return a % b;
  if (eq(op, "<")) return a < b;
  if (eq(op, ">")) return a > b;
  if (eq(op, "==")) return a == b;
  if (eq(op, "!=")) return a != b;
  if (eq(op, "<=")) return a <= b;
  if (eq(op, ">=")) return a >= b;
  error("irOp2: op=%s not found!", op);
}

int irExec(int i) {
  IR *p = &ir[i];
  IrOp op = p->op;
  int p0 = p->p, p1 = p->p1, p2 = p->p2;
  char *str = p->str;
  int pc = i + 1;
  int *vp, v, cond;
  trace("%02d: ", i);
  switch (op) {
    case IrAssignSt:
      vp = varLookup(str);
      *vp = t[p0];
      trace("%s = t%d = %d\n", str, p0, *vp);
      break;
    case IrAssignTs:
      v = (isdigit(*str)) ? atoi(str) : *varLookup(str);
      t[p0] = v;
      trace("t%d = %s = %d\n", p0, str, v);
      break;
    case IrLabel:
      trace("(L%d) = %d\n", p0, L[p0]);
      break;
    case IrGoto:
      pc = L[label];
      trace("goto L%d = %d\n", label, pc);
      break;
    case IrIfGoto:
      cond = t[p0];
      trace("if t%d(=%d) ", p0, cond);
      if (cond) {
        pc = L[label];
        trace("goto L%d = %d\n", label, pc);
      } else trace(" -- fail\n");
      break;
    case IrIfNotGoto:
      cond = t[p0];
      trace("ifnot t%d(=%d) ", p0, cond);
      if (!cond) {
        pc = L[label];
        trace("goto L%d = %d\n", label, L[label]);
      } else trace(" -- fail\n");
      break;
    case IrOp2:
      t[p0] = irOp2(t[p1], str, t[p2]);
      trace("t%d = t%d %s t%d = %d\n", p0, p1, str, p2, t[p0]);
      break;
    default: error("irExec: op=%d not found!", op);
  }
  return pc;
}

void irRun() {
  printf("===================irRun()=======================\n");
  mapNew(&varMap, VAR_MAX);
  for (int pc = 0; pc < irTop;) {
    pc = irExec(pc);
  }
}

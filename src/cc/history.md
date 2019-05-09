# cc Compiler

## 版本

1. 基本語法，包含 while, assign, exp, ...
2. 進階語法，加入 if, call, return, ....
    * 但是本版不支援函數定義，只能用 asm 呼叫巨集組合語言。
3. 支援函數定義，有區域變數存取功能！
    * 接下來要改 ir2m，轉換 ir 時加入區域變數存取指令。

## 範例

add.cx

```
a=3;
b=5;
c = add(a,b);
asm(".puti c\n.exit");

def add(x, y) {
  return x+y;
}
```

add.ix

```
t1 = 3
a = t1
t1 = 5
b = t1
t1 = a
t2 = b
arg t1
arg t2
call add
c = t0
asm ".puti c\n.exit"
function add
param x
param y
t1 = x
t2 = y
t3 = t1 + t2
return t3
fend
```

add.mx

```
t1 = 3
a = t1
t1 = 5
b = t1
t1 = a
t2 = b
arg t1
arg t2
call add
c = t0
asm ".puti c\n.exit"
function add
param x // x = P1
param y // y = P2
local i
t1 = x // t1 = P1    // addr param 1 p;  pget p t1
t2 = y // t2 = P2    // addr param 2 p;  pget p t2
t3 = t1 + t2
i=t3   // L1 = t3    // addr local 1 p;  pset p t3
return t3 
fend
```

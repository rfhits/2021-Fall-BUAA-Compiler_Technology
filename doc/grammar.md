# 文法

```cpp
编译单元: CompUnit → {Decl} {FuncDef} MainFuncDef

声明: Decl → ConstDecl | VarDecl

常量声明: ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'

基本类型 BType → 'int'

常数定义 ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal

常量初值 ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    
变量声明 VarDecl → BType VarDef { ',' VarDef } ';' 
    // 1.花括号内重复0次 2.花括号内重复多次
    
变量定义 VarDef → Ident { '[' ConstExp ']' } | 
    			Ident { '[' ConstExp ']' } '=' InitVal
	// 包含普通变量、⼀维数组、⼆维数组定义

变量初值 InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}'
    // 1.表达式初值 2.⼀维数组初值 3.⼆维数组初值
    
函数定义 FuncDef → FuncType Ident '(' [FuncFParams] ')' Block 
    // 1.⽆形参 2.有形参

函数类型 FuncType → 'void' | 'int' 
    // 覆盖两种类型的函数

函数形参表 FuncFParams → FuncFParam { ',' FuncFParam } 
	// 1.花括号内重复0次 2.花括号内重复多次

函数形参 FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }] 
    // 1.普通变量2.⼀维数组变量 3.⼆维数组变量
    
主函数定义 MainFuncDef → 'int' 'main' '(' ')' Block 
    // 存在main函数

语句块 Block → '{' { BlockItem } '}' 
    // 1.花括号内重复0次 2.花括号内重复多次
    
语句块项 BlockItem → Decl | Stmt
语句 Stmt → LVal '=' Exp ';' |
         	[Exp] ';' //有⽆Exp两种情况
        | Block
        | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] // 1.有else 2.⽆else
        | 'while' '(' Cond ')' Stmt
        | 'break' ';' | 'continue' ';'
        | 'return' [Exp] ';' // 1.有Exp 2.⽆Exp
        | LVal = 'getint''('')'';'
        | 'printf''('FormatString{,Exp}')'';' // 1.有Exp 2.⽆Exp
    
表达式 Exp → AddExp 注：SysY 表达式是int 型表达式 // 存在即可
条件表达式 Cond → LOrExp // 存在即可
左值表达式 LVal → Ident {'[' Exp ']'} //1.普通变量 2.⼀维数组 3.⼆维数组
基本表达式 PrimaryExp → '(' Exp ')' | LVal | Number // 三种情况均需覆盖
数值 Number → IntConst // 存在即可
⼀元表达式 UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' // 3种情况均需覆盖,
函数调⽤也需要覆盖FuncRParams的不同情况
| UnaryOp UnaryExp // 存在即可
单⽬运算符 UnaryOp → '+' | '−' | '!' 注：'!'仅出现在条件表达式中 // 三种均需覆盖
函数实参表 FuncRParams → Exp { ',' Exp } // 1.花括号内重复0次 2.花括号内重复多次 3.
Exp需要覆盖数组传参和部分数组传参
乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp //
1.UnaryExp 2.* 3./ 4.% 均需覆盖
加减表达式 AddExp → MulExp | AddExp ('+' | '−') MulExp // 1.MulExp 2.+ 需覆盖 3.-
需覆盖
关系表达式 RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp // 1.AddExp
2.< 3.> 4.<= 5.>= 均需覆盖
相等性表达式 EqExp → RelExp | EqExp ('==' | '!=') RelExp // 1.RelExp 2.== 3.!=
均需覆盖
逻辑与表达式 LAndExp → EqExp | LAndExp '&&' EqExp // 1.EqExp 2.&& 均需覆盖
逻辑或表达式 LOrExp → LAndExp | LOrExp '||' LAndExp // 1.LAndExp 2.|| 均需覆盖
常量表达式 ConstExp → AddExp 注：使⽤的Ident 必须是常量 // 存在即可
```




# 编译原理

2021秋，由北京航空航天大学计算机学院开设的编译技术课程。

keywords: BUAA 北航 编译 编译器

最终6个testfile竞速排名为：13，22，21，13，37，8。

testfile1是公开的，没啥意思，不想太卷。

testfile2有**除法**没被优化，不知道是哪个除数，遗憾。

testfile5不知道优化点在哪，寄了。

## 考题

### 期中考

是新增char类型的语法分析，难点在找出FIRST包含char的非终结符。

### 期末考

新增对ConstExp的判断，必须是整数，可以在10行之内搞定。

新增i++，i--的代码生成。`<Statement> --> <Identifier>++|<Identifier>--`，20行之内可以解决。

分析一个函数的活动记录，以及从下列三个优化中选一个说说自己的实现。

1. 图着色
2. 临时寄存器分配
3. DAG

3个新的代码优化样例，要求提交优化前/后、中间/目标代码，共12个txt。建议对于每个testfile能够直接输出这四个文件，毕竟在Win7上重命名12个文件是一件非常糟心的事情。

## 仓库描述

### 分支

lexer: 词法分析，要求输出词素

parser: 语法分析，要求递归输出语法树节点

handle_error: 错误处理，要求建立符号表，进行初步的语义分析

intermediate: 中间代码、目标代码生成

main: 完成了各种优化，最终的优化代码

错误处理上机挂了一个点，建议参考`intermediate`和`main`两个分支，前面的分支有隐藏bug。

### 遗留bug

因为经常是在当前分支写着写着就发现前面的分支有bug，所以就导致bug的遗留。

每一个分支中的bug，会在前面所有分支中出现，建议直接查看`intermediate`和`main`下的内容

  + lexer: get_token最后没有更新token的行号，此bug在handle_error中解决
  + parser: 一个分号和逗号搞反了，此bug在handle_error中解决
  + handle_error: 
    + 对错误发生的if-else出现问题
    + `MulExp()`乘除法反了；
    + 对函数的数组传参理解出现错误，我以为是拷贝整个数组，结果是传地址；
    + 对逻辑短路理解出现错误；
    + ...


## 开发建议

开发的时候因为思考不周到犯了很多错误，导致后续花费了大量的时间来修正。罗列一些建议，有空再来补上

1. `Lval = sth`和`Lval = Lval`要区分开，做一个`AssignedLVal`，因为它们词法结构虽然相同，但是语义动作是不同的。
2. 目标代码生成时，分两次完成。一次只用四个寄存器，这个版本用来上机，另一次就得图着色。
3. ...

## 感恩

学长们的代码在风格、注释、架构上都起了很大帮助，感谢他们。

感谢我的同学csh，教会了我图着色。
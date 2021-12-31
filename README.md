# 编译原理

2021秋，由北京航空航天大学计算机学院开设的编译技术课程。

keywords: BUAA 北航 编译 编译器

## 考核方式

+ 实践部分(40%）：上机实践(48 机时)
+ 理论基础（60%）:课堂教学，按时交作业。
    - 作业10分
    - 3-6次随堂考试，共计30分，不补考
    - 期末闭卷考试，60分
    - 主动回答问题，每次奖励0.5分，5分封顶(考前公布)


## 考题

期中考和期末考都不难。

### 期中考

是新增char类型的语法分析，难点在找出FIRST包含char的非终结符。

### 期末考

新增对ConstExp的判断，必须是整数。

新增i++，i--的代码生成。`<Statement> --> i++|i--`

3个新的代码优化样例，要求提交优化前后中间目标代码。

## 仓库描述

### 分支

lexer: 词法分析，要求分割出词素

grammatical_analysis: 语法分析，要求递归输出语法树

handle_error: 错误处理，要求建立符号表，进行初步的语义分析

intermediate: 中间代码生成

graph-coloring: 完成图着色，最终的上机代码

grammatical_analysis: 语义分析

每个分支都有隐藏的bug，所以建议参考`intermediate`和`graph-coloring`两个分支。


### 开发建议

`Lval = sth`和`Lval = Lval`要区分开，做一个`AssignedLVal`，因为它们词法结构虽然相同，但是语义动作是不同的。



### 遗留bug

每一个分支中的bug，会在前面所有分支中出现，所以，请直接查看base下的内容

  + lexer: get_token最后没有更新token的行号，此bug在handle_error中解决
  + parser: 一个分号和逗号搞反了，此bug在handle_error中解决
  + handle_error: 
    + 对错误发生的if-else出现问题
    + `MulExp()`乘除法反了，在base中解决
    + 对函数的数组传参理解出现重大失误，我以为是拷贝整个数组，结果是传地址，在base中解决
    + 对逻辑短路理解出现重大失误


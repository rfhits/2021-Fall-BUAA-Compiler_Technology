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

## 仓库描述

### 分支

#### lexer

词法分析，有bug，get_token最后没有更新token的行号。
语法分析，错误处理的预留接口有问题，应当是突然insert一个if(not expected)，而不是发生错误就不处理了

### 开发建议

`Lval = sth`和`Lval = Lval`要区分开，做一个`AssignedLVal`，因为它们词法结构虽然相同，但是语义动作是不同的。





### 遗留bug

每一个分支中的bug，会在前面所有分支中出现，所以，如果想要de掉lexer中的bug，需要将

  + lexer: get_token最后没有更新token的行号，此bug在handle_error中解决
  + parser: 一个分号和逗号搞反了，此bug在handle_error中解决
  + handle_error: 


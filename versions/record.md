各个版本的记录

base：

没有任何的优化，寄存器的分配还是错误的

no-reduant-reg:

对于 `add a1 a2 a3`这样的指令，不会再把a1从内存中取出来了

不知道lru写对没有

while-cond-out:

cond外移，快了10分之一到200分之一之间，，没用啊。


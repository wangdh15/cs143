## 1. 检查继承图，确保继承图合法
1. 图中不存在环
2. 所有父节点都被定义了（A 继承了 B，但是B没有被定义，这个时候也是没有环的）
3. 对于自带的四个类
    - IO: 可以被继承，不能被重新定义
    - Int: 不可被继承，不可被重新定义
    - String: 不可以被继承，不可以被重新定义
    - Bool: 不可以被继承，不可以被重新定义
4. 不可以继承SELF_TYPE
5. 文件中一定要包含Main类

所有的Class也不能被重新定义

首先直接检查条件2和条件3，然后利用拓扑排序的方法检查条件1是否满足即可。

每个class中包含四个部分：
- name : Symbol
- parent: Symbol
- features: Features
- filename: Symbol

## 2. 检查其他语义条件

Type Environment:
1. Object Environment (O), 记录当前环境中各个符号的类型。（以Symbol为key，以Symbol为Value）输入变量名，输出该变量的类型。
2. Method Environment (M), 记录每个类的每个方法的形参列表（形参名称和类型）、返回值类型。
3. C，当前执行所在的类。

对于第二个，需要先扫描一遍所有的类以及他对应的method的函数签名，构造出Method Environment
对于第一条，只需要维护一个符号表即可。

在遍历一个AST的时候，如下的表达式会引入新的scope：
1. 当前检查的类定义的属性。
2. 一个方法的形式参数。
3. let表达式。
4. case分支。

对于第三个，由于在检查的过程中是一个类、一个函数进行检查的。
所以只需要维护一个变量表示当前的类即可。当进入一个新的类里面就
更新这个变量。


### 2.1 类型检查

主要的目的是给AST的每个表达式节点都分配一个类型。

对于每个表达式节点，其检查的步骤如下：
1. 首先对条件进行检查，递归调用，并获取对应依赖的子表达式的类型。以及类型要求的条件。
2. 处理计算返回当前表达式的类型。

同时对于每一个函数，还需要检查其内部expr的类型和
其申明的返回值类型相匹配。

如果一个函数内部发生了对另外一个函数的调用，则只需要比较传入的类型，以及申明的类型
是否匹配即可。

检查的内容：
1. 子类不可以重写父类的属性。
2. 子类可以重写父类的方法，但是方法的形参个数、形参类型以及返回值类型必须要完全一致。


1. class__class 检查的逻辑：
    - 将当前类的attr加入到namespace中（包括自己父类的）
    - 检查自己的attr的合法性
    - 检查自己的method的合法性

2. attr__class 检查逻辑：
    （需要加入self关键字）
    - 获取expr的类型
    - 检查expr的类型和自己声明的类型是否符合条件（表达式类型 <= 声明类型）

3. method__class 检查逻辑：
    （需要加入self关键字）
    - 获取expr的类型
    - 检查expr的类型和自己的返回类型是否符合条件

4. assign 检查逻辑：
    - 根据当前环境获取到对应的name的声明类型
    - 获取expr的类型
    - 检查expr的类型和name声明类型的合法性

5. bool_const_class:
    - 直接返回Bool类型

6. int_const_class:
    - 直接返回Int类型

7. string_const_class:
    - 直接返回String类型

8. new__class:
    - 如果type_name为SELF_TYPE，则返回C
    - 否则的话，返回type_name.(这里需不需要判断type存在？)

9. dispatch_class:
    - 获取expr的类型
    - 获取actual的类型
    - 根据expr的类别，获取调用对象的类型 （SELF_TYPE）
    - 获取name对应的函数签名
    - 检查actubal的个数以及类型和函数签名是否匹配
    - 根据函数签名的类比，返回对应的类型（SELF_TYPE）

10. static_dispatch_class:
    - 获取expr的类型
    - 获取actual的类型
    - 判断expr的类型和type_name的类型符合要求
    - 根据type_name和name获取对应的函数签名
    - 判断actual的类型和函数签名的类型是否匹配
    - 根据函数签名返回值类型，返回对应的类型（SELF_TYPE）

11. cond_class:
    - 获取pred的类型，并检查是否为Bool
    - 获取then_exp的类型，else_exp的类型
    - 求得then_exp和else_exp的最近公共祖先，作为返回值类型

12. block_class:
    - 逐个获取body的各个表达式的类型
    - 返回最后一个表达式的类型

13. let_class:
    - 获取init的类型（无表达式的话，算都合法）
    - 获取type_decl的类型（需要考虑SELF_TYPE）
    - 检查type_decl和init类型是否匹配
    - 增加一个新的scope，将identifier加入
    - 获取body的类型
    - 返回body的类型

14. typcase_class:
    - 获取到expr的类型
    - 获取到各个cases的类型
    - 求这些cases的最近公共祖先，作为返回的类型

15. branch_class:
    - 进入一个新的scope，将name类型指定为type_decl，
    - 获取expr的类型
    - 退出scope，返回expr的类型

16. loop_class:
    - 获取pred的类型，确保为Bool
    - 获取body的类型
    - 返回Object类型

17. isvoid_class:
    - 检查e1的类型
    - 返回Bool

18. comp_class:
    - 检查e1的类型，确保为Bool
    - 返回Bool

19. lt_class, leq_class:
    - 检查e1的类型，确保为Int
    - 检查e2的类型，确保为Int
    - 返回Bool

20. neg_class:
    - 检查e1的类型，确保为Int
    - 返回Int

21. add_class, minus_class, mul_class, divide_class:
    - 检查e1, e2类型，确保为Int
    - 返回类型Int

22. eq_class:
    - 检查e1类型，e2类型
    - T1, T2在 {Int, String, Bool}之中，且T1 == T2
    - 返回Bool

23.

 comp_class:
    - 获取e1的类型，检查是否为Bool
    - 返回Bool类型

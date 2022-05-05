## 评测脚本的坑

1. 评测脚本默认spim的路径是`/usr/class/cs143/bin/spim`，所以至少需要在这个目录下创个软连接
2. 评测脚本暴力匹配输出和他自己存的数据，而自己存的加载的lib的路径不一样，所以在执行完paxx.pl之后，执行`python change.py`替换一个对应内容
3. 到`grading`目录下，执行`./143publicxx PA5`即可获得正确的评判结果

## runtime 需要的符号

见 cool-runtime 的Figure3,生成的代码需要在对应的位置存在这些符号，而且
需要时 .global 的

### class_nameTab
以class_tag作为索引，存放一些字符串对象的指针，每个字符串对象中包含着class的名字：
1. basic类
    - Object
    - IO
    - Int
    - String
    - Bool
2. Main类
3. 其他用户定义的类

### class_objTab
以class_tab作为索引，每个类有两个指针:
- 该类的protoObj
- 该类的init方法

### 一些对象和protObj和init方法

- Main_protObj
- Main_init

- Int_protObj
- Int_int

- String_protObj
- String_init

其他对象的protObj和init方法虽然runtime没有用到，但是还需要提供：
- IO_protObj：没有成员数据，只有一个函数表指针，具体函数的实现由runtime提供
- IO_init: 只需要调用父类（Object）的init方法

- Bool_protObj：只有一个成员，还有一个函数表指针，且函数表中没有自己的函数，全部是父类的函数
- Bool_init：调用父类的init方法即可。

C.m 每个类的函数需要有要给label


### 一些tab

- int_tag
- bool_tag
- string_tag

每个class有一个独一无二的tag，这些tag是class_nameTab和class_objTab的索引。
需要显示地申明上述的tag，然后将对应的tag值填入。
对于其他的class，只需要将其tag填充到对应的protObj中即可，不需要设立符号。

### bool_const0

一个bool对象，代表着bool的false

### GC相关的符号
- _MemMgr_INITIALIZER：指定GC的初始化代码
- _MemMgr_COLLECTOR：指定使用哪一个GC
- _MemMgr_TEST：如果设置为1且指定了GC的话，则每次内存分配都会触发一次GC，便于DEBUG


## runtime 提供的符号

见 cool-runtime 的Figure4, 这些符号的可以在生成的代码中直接使用，具体的
含义和参数见表

### Object的方法实现

- Object.copy
- Object.abort
- Object.type_name

### IO的方法实现

- IO.out_string
- IO.out_int
- IO.in_string
- IO.in_int

### String的方法的实现

- String.length
- String.concat
- String.substr


### 其他方法

- _dispatch_abort: 当尝试在一个void的对象上调用方法的时候，调用该方法
- _case_abort: 当case没有匹配上对应的分支的时候，调用该方法
- _case_abort2: 当case尝试在一个void对象上进行是
- _equality_test: 测试两个对象是不是Int,String,Bool中的一个，并且值相同。

### GC相关

- GenGC_Assign：每次对一个属性赋值的话，就调用一次这个函数，触发GC



## 可执行文件布局


## 实现细节

xx_tag:
Object: 0
IO: 1
String: 2
Int: 3
Bool: 4
Main: 5

## .text部分

对于.text的部分，需要实现的部分有：
1. 每个class的init函数（包括Object, IO, String, Int, Bool, Main）
2. 每个class的其他函数（不包括Object, IO, String, Int, Bool）


### xxx_init

每个class的init函数需要包含以下几个部分：
- 函数头
- 调用父类的init函数
- 初始化自己的attr
- 函数尾

也就是这部分负责执行每个class的attr部分的执行

### C.m

其他函数的定义

采用stack machine的机制，每个表达式的求值遵守如下的约定：
1. 求值前和求值后，栈的状态，还有其他寄存器的状态不变
2. 表达式的结果存储到 $a0中。

对于每个函数而言，其调用遵守如下的约定
- $a0 存储了 self 的值
- 最终的$a0存储了返回的值
- 跳转之前，caller需要将自己的fp、参数压入栈，栈的布局如下：
    + old fp
    + args n
    + args n - 1
    + ....
    + args 1
                <- sp

- 跳转之后，callee一开始需要将返回地址压栈，并更新fp，得到的布局如下：
    + olf fp
    + args n
    + args n - 1
    + ....
    + args 1
    + ret addr($ra)         <- fp
                            <- sp
- 然后利用fp + i * 4就可以获取到第i个参数

- callee返回的时候，栈的布局需要和caller调用的时候相同，且$a0存储这函数返回 的结果。这个时候callee负责将参数弹出，并恢复fp的值。

函数内部可以使用的变量有如下几类：
1. 当前类及其父类中包含的attr
2. 当前函数的参数
3. let表达式引入的变量
4. branch引入的变量

在函数内部，$s0用于存储self，可以用其和偏移量来访问attr
$fp用来访问参数、let和branch引入的变量

## 不同expr的操作

### assign_class
- 生成expr的代码
- 将$a0的值放到栈上
- 根据env和name获取变量对应的位置
- 将expr的返回结果赋值给对应的位置
- 将expr的返回结果放到$a0中

### static_dispatch_class

-

### typcase_class

- 每个分支创建一个label:
- 在每个分支入口，利用blt和bgt两个命令，来判断是否是子类（继承的classTag需要使用dfs序）
- 如果不满足，就跳到下一个label去
- 如果满足，符号表添加新增的变量，将这个变量放到栈上，然后执行对应分支的逻辑，将结果放到$a0上，然后b到结束的label即可。


# V2

## protObj和init的实现

- 按照文档要求，Int, String, Bool 三个类型protObj的对应的attr需要合适设定，其他类型的protObj的attr的值没有要求。
- 如果一个class的attr没有赋值的话，如果其类型为Int, String, Bool，则需要是对应的default的值。其他的属性需要时void(0)

所以在设计的时候，遵循了如下的标准：
1. 对应Int, String, Bool三种类型的protObj，按照文档设置好对应的attr。
2. 对于其他类型的protObj，如果其attr是Int, String, Bool的话，将其设定为对应的protObj类型（默认值）；如果是其他类型的话，则置为零。
3. 对于每个类的init方法，首先：
    - 调用父类的init方法
    - 检查每一个attr：
        + 如果init表达式是no_expr，则不需要进行处理（protoObj已经正确处理了）
        + 如果init表达式不是no_expr，则执行对应的initcode，然后赋值即可。

## 函数调用协议

1. 通过$a0传递self指针
2. 将参数按照顺序压栈，第一个参数先压栈，第二个参数再压
3. 将参数弹出栈的逻辑是再被调函数中执行的
4. 函数的返回值存放在$a0中，且fp寄存器，s0寄存器都不变

## equal判断

1. 首先判断两个对象的地址，如果地址相等，则两个对象相等
    + void只和void相等，其和其他都不相等
2. 则调用库提供的equality test函数即可。
    + 两个参数通过t1, t2传递，且a0和a1传递结果
    + 如果满足t1,t2类型都是Int,String,Bool,且值相等，则返回a0
    + 否则的话，返回a1

## case
对于case而言，要求在所有分支中找到和其最近的祖先。
所以在生成代码的时候，先将所有的分支按照在继承树中的深度进行排序，然后先生成深度较大的即可。

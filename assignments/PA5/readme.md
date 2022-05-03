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

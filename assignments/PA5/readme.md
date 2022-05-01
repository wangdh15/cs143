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

libparser
=========

解析静态库(Lib)文件，提取出所有函数信息，组织成自定义格式文件

1. LibParser 把Libs目录下的各个VC版本的CRT库(.lib)中的函数信息(函数名，函数数据)提取出来，再组织成自定义文件格式
   函数库文件(.flb)函数库文件格式：签名-函数头表-函数名称段-函数数据段

2. LibScanner 解析PE文件，把其中的CRT库函数的调用指令打印出来

测试例子，test.exe 结果文件test.log

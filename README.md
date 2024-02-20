# lshTiny
C 实现一套HTTP协议的服务端的学习型项目

- build 可执行文件产物
- cgi-bin 动态内容处理目录
- lib 引入的静态库
- lshTools 引入的工具库，链表实现字典
- objc .o文件存放目录
- resource 服务资源存放
- src 源文件存放目录



### 运行项目环境： macOS 13.6.1  vscode： 1.86.0

#### 项目怎样跑起来：  

Note: 生成的可执行文件最好都给下权限 chomd +x  可执行文件

1、lshTools 目录下执行make 命令，生成 liblshTools.a 库

2、回到根目录lshTiny 执行 make 命令，生成 server可执行文件

3、配置vscode，即可debug 该项目src/server.c文件,  ![截屏2024-02-20 15.27.18](/Users/fu-mobile/Library/Application Support/typora-user-images/截屏2024-02-20 15.27.18.png)

Launch.json 已经配置好。

4、如果想执行动态内容，需要进入cgi-bin目录下在执行make 生成 add 和 addForm 可执行文件。


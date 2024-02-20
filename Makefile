
CC = clang

CFLAGS = -Wall -m64 -g -O0

#指定静态库.a文件路径
LIBRARY = lib/liblsh.a lshTools/liblshTools.a

#指定头文件搜索路径为 lib目录 lshTools目录 。。。
INCLUDE_PATH = -Ilib -IlshTools

#源文件目录
SRCDIR = src
#可执行文件目录
BUILDDIR = build
#目标文件目录
OBJCDIR = objc

#指定源文件目录下需要处理的源文件
SRCS = $(wildcard $(SRCDIR)/*.c)

#将所有的.c 编译成对应的.o: SRCS:srcPath=desPath 指定需要编译的源文件:源文件所在目录=目标文件所在位置
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJCDIR)/%.o)

#指定可执行文件
EXEC = $(BUILDDIR)/server

all: $(EXEC) 

# $(OBJS) 表示生成可执行文件$(EXEC)需要的依赖， $(LIBRARY) 表示生成可执行文件需要依赖的库
# $(CC) $(CFLAGS)指定编译器和编译选项
#-L. . 表示当前目录，-Llib 表示 ./lib  -llsh 表示静态库 liblsh.a, 
#$^ 展开所有的.o 文件列表 $@表示所有目标文件路径即 $(EXEC) 里面的BUILDDIR
$(EXEC): $(OBJS) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -llsh -LlshTools -llshTools

#s$(OBJCDIR)/%.o 是规则目标，$(SRCDIR)/%.c 规则的前提或者依赖  $(CC) $(CFLAGS)指定编译器和编译选项
#-c 表示告诉编译器生产对象文件，但不进行链接，编译阶段使用
#$< 表示编译的源文件路径即 $(SRCDIR)
#$@ 表示生产的.o文件路径即 
$(OBJCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH)

clean: 
	rm -rf ./objc/*.o ./build/server 
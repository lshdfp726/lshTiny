
CC = clang

CFLAGS = -Wall -m64 -g -O0

#指定静态库.a文件路径
LIBRARY = ../lib/liblsh.a

#指定头文件搜索路径为 lib目录
INCLUDE_PATH = -I../lib  

OBJS = add.o

# all: server 

#-L.  . 表示当前目录，-Llib 表示 ./lib 
add: $(OBJS) $(LIBRARY)
	$(CC) $(CFLAGS) -o add $(OBJS) -L../lib -llsh 
#server.o 是规则目标， server.c 规则的前提或者依赖  $(CC) $(CFLAGS)指定编译器和编译选项
#-c 表示告诉编译器生产对象文件，但不进行链接，编译阶段使用
add.o: add.c
	$(CC) $(CFLAGS) -c add.c $(INCLUDE_PATH)

clean: 
	rm -rf *.o add 
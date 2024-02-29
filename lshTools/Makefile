
CC = clang

CFLAGS = -Wall -m64 -g -O0

#静态库名称
LIBRARY = liblshTools.a

#指定头文件搜索路径为 lib目录
INCLUDE_PATH = -I../lshTools  

#所有源文件
SRCS = lshDict.c
#打lshIO 和lshSocket 静态库包
OBJS = $(SRCS:.c=.o)

# all: server 

# $(LIBRARY): $(OBJS)
# 	ar rcs $@ $(OBJS)
	
lshDict: $(OBJS) 
	$(CC) $(CFLAGS) -o lshDict $(OBJS)

#server.o 是规则目标， server.c 规则的前提或者依赖  $(CC) $(CFLAGS)指定编译器和编译选项
#-c 表示告诉编译器生产对象文件，但不进行链接，编译阶段使用
lshDict.o: lshDict.c
	$(CC) $(CFLAGS) -c lshDict.c $(INCLUDE_PATH)

clean: 
	rm -rf *.o lshDict *.a
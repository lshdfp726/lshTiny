#ifndef LSHSOCKET_H
#define LSHSOCKET_H

#define MAXBUF 1024

int lsh_nslookup(char *src);

//客户端连接服务器
int lsh_openClientfd(char *hostname, char *port);
//服务端绑定和监听
int lsh_openListenfd(char *port);

//ip地址16进制数字转为点分10进制
int hex2dd(char *src);
//ip地址点分10进制转为16进制数字
int dd2hex(char *src);
#endif
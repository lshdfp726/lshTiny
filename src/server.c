#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#include "lshIO.h"
#include "lshSocket.h"
#include "lshDict.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct stat stats;


#define ENDLINE "\r\n"

extern char **environ;

//执行服务端业务入口
void doit(int fd);
//报错打印
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
//打印并返回请求头
void readCache_requesthdrs(lshRio_t *rp, Dictionary *dict);

//解析 uri参数
int parse_uri(char *uri, char *filename, char *cgiargs);
//处理静态内容
void serve_static(int fd, char *filename, int filesize);
//获取文件类型
void get_filetype(char *filename, char *filetype);
//处理动态内容
void serve_dynamic(int fd, char *filename, char *cgiargs);

//HEAD 方法
void serve_head(int fd, char *filename, int filesize);

//POST 方法 fd socket 客户端句柄，filename，要处理的文件名称 postContent 请求的内容
void server_post(int fd, char *fileName, const char *postContent);

//验证套接字是否可写
int writable_fd(int fd);

//匹配HTTP 方式
int foundMethod(char *method);

//段错误异常捕获
void sigsegv_handle(int signo) {
    fprintf(stderr, "Segmentation fault (signal %d)\n",signo);
    //忽略因为通道关闭导致异常退出，所以注释
    // exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    int listenFd, connFd;
    char hostName[MAXBUF], port[MAXBUF];
    socklen_t clientLen;
    sockaddr_storage clientAddr;

    if (argc != 2) 
    {
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }

    //socket pipe关闭还继续写导致的异常信号捕获
    signal(SIGPIPE, sigsegv_handle);

    listenFd = lsh_openListenfd((char *)argv[1]);

    /**
     * 正常情况下 while 循环会被accept阻塞
     */
    while (1)
    {
        clientLen = sizeof(clientAddr);
        connFd = accept(listenFd, (sockaddr *)&clientAddr, &clientLen);
        if (connFd < 0) {
            printf("accpet failed connfd: %d\n",connFd);
            continue;
        } 
        printf("accept success\n");
        //提取clientAddr 里面的hostName和port
        getnameinfo((sockaddr *)&clientAddr, clientLen, hostName, MAXBUF, port, MAXBUF, 0);
        printf("getnameinfo success\n");
        printf("Accept connect from (%s, %s)\n", hostName, port);
        if (writable_fd(connFd) < 0) {
            printf("套接字已经关闭，不可写，重新accept\n");
            continue;
        }
        doit(connFd);
        //根据客户端头字段connection: close 来关闭，暂时不处理
        close(connFd);
    }

    return 0;
}

void doit(int fd) {
    int is_static; //是否是静态内容，用于GET 方法
    stats sbuf; //文件元数据描述对象
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[MAXBUF];
    char fileName[MAXBUF], cgiargs[MAXBUF];
    lshRio_t rio; //对unix IO的一层薄封装

    lshRio_readinitb(&rio, fd);
    lshc_readline(&rio, buf, MAXBUF);
    printf("Request headers:\n");
    printf("%s\n",buf);

    // lsh_writen(fd, buf, strlen(buf));

    sscanf(buf, "%s %s %s",method, uri, version);
    
    if (foundMethod(method) < 0)
    {
        clienterror(fd, method, "501", "Not implemented", "Tiny dose not implement this method");
        return ;
    }

    Dictionary dict;
    init_dictionary(&dict);
    readCache_requesthdrs(&rio, &dict);

    is_static = parse_uri(uri, fileName, cgiargs);
    if (stat(fileName, &sbuf) < 0) 
    {
        clienterror(fd, fileName, "404", "Not found", "Tiny couldn't find this file");
        return ;
    }

    if (strcasecmp(method, "HEAD") == 0) 
    {
        serve_head(fd, fileName, sbuf.st_size);
    }
    else if (strcasecmp(method, "GET") == 0) 
    {
        if (is_static) 
        {
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) //非正常文件或者没有读权限
            {
                clienterror(fd, fileName, "403", "Forbidden", "Tiny couldn't read the file");
                return ;
            }
            serve_static(fd, fileName, sbuf.st_size);
        } 
        else 
        {
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) //非正常文件或者没有执行权限
            {
                clienterror(fd, fileName, "403", "Forbidden", "Tiny couldn't run the CGI program");
                return ;
            }
            serve_dynamic(fd, fileName, cgiargs);
        }
    } 
    else if (strcasecmp(method, "POST") == 0) 
    {
        /**
         *  可以终端使用telnet 来模拟post 请求：
         *  POST /resource/post.txt HTTP/1.1
            host:localhost
            Content-type:application/json
            Content-Length: 15

            name=john&age=35

         *  这里省事就不处理 Content-type 对应的各种类型了，只单纯获取到 name=john&age=35 写入文件 /resource/post.txt 里面
         */
        const char *contentLength = find_entry(&dict, "Content-Length");
        int length = atoi(contentLength);
        char postContent[length];
        lshc_readline(&rio, postContent, length);
        server_post(fd, fileName, postContent);
    }
    return ;
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)  
{
    char buf[MAXBUF], body[MAXBUF];

    /** build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error <title>");
    sprintf(body, "%s<body bgcolor=\"ffffff\">%s",body, ENDLINE);
    sprintf(body, "%s%s: %s%s",body, errnum, shortmsg, ENDLINE);
    sprintf(body, "%s<p>%s: %s%s",body, longmsg, cause,ENDLINE);
    sprintf(body, "%s<hr><em>The Tiny Web server%s", body,ENDLINE);

    /** Print the HTTP response*/
    sprintf(buf, "HTTP/1.0 %s %s%s", errnum, shortmsg,ENDLINE);
    lsh_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html%s",ENDLINE);
    lsh_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d%s%s",(int)strlen(body),ENDLINE,ENDLINE);
    lsh_writen(fd, buf, strlen(buf));
    lsh_writen(fd, body, strlen(body));
}
                 
void readCache_requesthdrs(lshRio_t *rp, Dictionary *dict) 
{
    char buf[MAXBUF];
    lshc_readline(rp, buf, MAXBUF);
    while (strcmp(buf, ENDLINE)) //strcmp(str1, str2) str1和str2相等时为0 退出，其他情况一直执行循环里面逻辑
    {
        lshc_readline(rp, buf, MAXBUF);
        printf("%s", buf);
        char key[MAXBUF], value[MAXBUF];
        sscanf(buf, "%[^:]:%[^\n]", key, value);
        add_entry(dict, key, value);
    }
}


int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;
    if (!strstr(uri, "cgi-bin")) //cgi-bin 表示是动态内容根目录，
    {
        strcpy(cgiargs, ""); //清除cgiargs内容
        strcpy(filename, "..");
        strcat(filename, uri); //uri 追加到filename 末尾，直到uri遇到\0位置，该函数并不检查filename 缓冲区长度。
        if (uri[strlen(uri) - 1] == '/')  //uri 缺省名称为/ 在这里解释称home.html, 当然也可以解释称任何合法的路径。具体看不同服务端的需求
            strcat(filename, "home.html");
        return 1;
    }
    else 
    {
        ptr = index(uri, '?');
        if (ptr) 
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

// 处理HEAD请求，只发送响应头
void serve_head(int fd, char *filename, int filesize) {
    char buf[MAXBUF];

    // 发送响应头信息
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: text/html\r\n\r\n", buf);

    lsh_writen(fd, buf, strlen(buf));
}

void server_post(int fd, char *fileName, const char *postContent) 
{
    int srcfd;
    char fileType[MAXBUF], buf[MAXBUF];

    get_filetype(fileName, fileType);
    sprintf(buf, "HTTP/1.0 200 OK%s",ENDLINE);
    sprintf(buf, "%sServer: Tiny Web Server%s", buf, ENDLINE);
    sprintf(buf, "%sConnection: close%s",buf,ENDLINE);
    sprintf(buf, "%sContent-type: %s%s%s",buf,fileType,ENDLINE,ENDLINE);

    lsh_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    srcfd = open(fileName, O_WRONLY, 0);
    if (lsh_writen(srcfd,(void *)postContent, strlen(postContent)) < 0) 
    {
        fprintf(stderr, "server_post write error");
    }
    close(srcfd);
}


void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, fileType[MAXBUF], buf[MAXBUF];

    /**Send response headers to client */
    get_filetype(filename, fileType);
    sprintf(buf, "HTTP/1.0 200 OK%s",ENDLINE);
    sprintf(buf, "%sServer: Tiny Web Server%s", buf, ENDLINE);
    sprintf(buf, "%sConnection: close%s",buf,ENDLINE);
    sprintf(buf, "%sContent-length: %d%s",buf, filesize, ENDLINE);
    sprintf(buf, "%sAccept-ranges: bytes%s",buf,ENDLINE); //支持流式传送和断续传
    // sprintf(buf, "%sContent-Disposition: attachment%s",buf,ENDLINE);//attachment 客户端支持下载 inline, 浏览器直接播放
    sprintf(buf, "%sContent-type: %s%s%s",buf,fileType,ENDLINE,ENDLINE);

    lsh_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s",buf);

    /** Send response body to client */
    srcfd = open(filename, O_RDONLY, 0);

    // srcp = (char *)malloc(filesize);
    // printf("strlen(srcp) %lu\n",strlen(srcp));
    // lsh_readn(srcfd, srcp, filesize);
    
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    // if (srcp) free(srcp);

    lsh_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpg");
    else if (strstr(filename, ".mpg"))
        strcpy(filetype, "video/mpeg");
    else 
        strcpy(filetype, "text/plain");
}


void sigchild_handler(int signo) {
    //信号处理编号ex:SIGINT 表示中断
    printf("signo is %d\n",signo); 
    int status;
    //循环等待所有子进程的退出状态， -1 表示任何子进程，> 0 表示特定子进程
    //调用函数WNOHANG非阻塞等待(理解为异步执行)
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d.\n",pid, WEXITSTATUS(status)) ;
        } else if (WIFSIGNALED(status)) {
            printf("Child process %d terminated by signal %d.\n", pid, WTERMSIG(status));
        } else {
            printf("Child process %d did not exit normally.\n", pid);
        }
    }

    if (pid < 0 && errno != ECHILD) perror("waitpid");
}

void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXBUF], **emptyList = {NULL};

    /** return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK%s",ENDLINE);
    lsh_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server%s",ENDLINE);
    lsh_writen(fd, buf, strlen(buf));


    pid_t pid = fork();

    signal(SIGCHLD, sigchild_handler);
    if (pid == 0) //子进程
    { 
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO); //标准输出重定向到文件流fd
        
        execve(filename, (char * const * )emptyList, environ);

    } 
    else if (pid == -1) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else {
       waitpid(pid, NULL, 0);
    }
}

int writable_fd(int fd) {
    fd_set write_fds;
    struct timeval timeout;
    FD_ZERO(&write_fds);
    FD_SET(fd, &write_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int ret = select(fd + 1, NULL, &write_fds, NULL, &timeout);
    return ret;
}

int foundMethod(char *method) 
{
    static char *supportMethod[] = {"GET","HEAD", "POST"};
    int count = sizeof(supportMethod) / sizeof(supportMethod[0]);
    for (int i = 0; i < count; i ++)  
    {
        char *m = supportMethod[i];
        if (strcasecmp(method, m)) 
        {
            return 0;
        }
        
    }
    return -1;
}
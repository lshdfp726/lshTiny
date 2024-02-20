
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "lshSocket.h"

#define MAXBACKBLOG 1024

int lsh_nslookup(char *src) {
    struct addrinfo *p, *listp, hints;
    char buf[MAXBUF];
    int rc, flags;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    /**
     * @brief 
     * @param host  主机域名/ip
     * @param service 服务器域名/ip
     * @param hints 辅助 struct addrinfo结构体，配置 family、socktype这些
     * @param result 
     * @return int 
     * 
     *    int getaddrinfo(const char *host, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **result);
     */

    if ((rc = getaddrinfo(src, NULL, &hints, &listp)) != 0) 
    {
        fprintf(stderr, "getaddrinfo  error %s\n", gai_strerror(rc));
        return -1;
    }

    flags = NI_NUMERICHOST;

    /**
     * @brief 
     * 
     * @param sa 指向抽象结构sockadd 
     * @param salen sizeof(sockadd)
     * @param host 本机 域名/ip
     * @param hostlen  
     * @param service 服务器域名/ip
     * @param servlen 
     * @param flags 
     * @return * int 
     * int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, size_t hostlen,
                char *service, size_t servlen, int flags);
     
     */
     
    for (p = listp; p; p = p->ai_next)
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXBUF, NULL, 0, flags);
        printf("%s\n",buf);
    }
    
    freeaddrinfo(listp);
    return 1;
}

int lsh_openClientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_socktype = SOCK_STREAM; // 打开套接字连接
    hints.ai_flags = AI_NUMERICSERV; // 使用数字端口
    hints.ai_flags |= AI_ADDRCONFIG;
    getaddrinfo(hostname, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next) 
    {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue ;
        }

        if (connect(clientfd, p->ai_addr, p ->ai_addrlen) != -1) 
            break;
        
        close(clientfd); //失败就关闭当前文件描述符
    }
    
    freeaddrinfo(listp);
    if (!p) 
        return -1; //所有链接失败
    else    
        return clientfd;
}

int lsh_openListenfd(char *port) {
    struct  addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next)
    {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;
        
        //ip地址复用
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        
        if (bind(listenfd, p ->ai_addr, p ->ai_addrlen) == 0)
            break;
        close(listenfd);
    }

    freeaddrinfo(listp);
    if (!p) 
        return -1;
    
    //主动套接字描述符转为监听描述符
    if (listen(listenfd, MAXBACKBLOG) < 0) {
        close(listenfd);
        return -1;
    }

    return listenfd;
}

int dd2hex(char *src) {
    // if (strlen(src) <= 0)
    //     fprintf(stderr, "dd2hex params: src invaild");
    //     return -1;
    
    struct in_addr inaddr;
    int rc;
  
    rc = inet_pton(AF_INET, src, &inaddr);

    if (rc == 0)
        fprintf(stderr, "inet_pton error: invalid dotted-decimal address");
    
    else if (rc < 0) 
        perror("inet_pton");

    printf("%u\n", ntohl(inaddr.s_addr));
    
    return 0;
}

int hex2dd(char *src) {
    // if (strlen(src) <= 0)
    //     fprintf(stderr, "hex2dd params: src invaild");
    //     return -1;
    
    struct in_addr inaddr;
    uint32_t addr;
    char buf[MAXBUF];
  
    sscanf(src, "%x", &addr);
    inaddr.s_addr = htonl(addr);

    if (!inet_ntop(AF_INET, &inaddr, buf, MAXBUF)) 
        perror("inet_ntop");

    printf("%s\n",buf);
    
    return 0;
}

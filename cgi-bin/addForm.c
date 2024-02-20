#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "lshIO.h"

#define ENDLINE "\r\n"
#define MAXBUF 1024
typedef struct stat stats;

int main(int argc, char const *argv[])
{
    char content[MAXBUF];

    /* Make the response body */
    char *filename = "./cgi-bin/addResponse.html";
    int fd = open(filename, O_RDONLY, 0);

    stats sbuf;
    if (stat(filename, &sbuf) < 0) 
    {
        printf("stat faild\n");
        exit(0);
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) //非正常文件或者没有读权限
    {
        printf("couldn't read the file\n");
        exit(0);
    }

    lsh_readn(fd, content, sbuf.st_size);
    /* Generate the HTTP response */
    printf("Connection: close%s",ENDLINE);
    printf("Content-length: %d%s", (int)strlen(content), ENDLINE);
    printf("Content-type: text/html%s%s",ENDLINE,ENDLINE);
    printf("%s", content);
    // fflush(stdout);

    exit(0);
}

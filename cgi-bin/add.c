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
    char *buf, *p;
    char arg1[MAXBUF], arg2[MAXBUF], content[MAXBUF];
    int n1 = 0, n2 = 0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }

    sprintf(content, "QUERY_STRING=%s",buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.%s<p>", content, ENDLINE);
    sprintf(content, "%sThe answer is: %d + %d = %d%s<p>",
            content, n1, n2, n1 + n2,ENDLINE);
    sprintf(content, "%sThanks for visiting!%s", content,ENDLINE);

    /* Generate the HTTP response */
    printf("Connection: close%s",ENDLINE);
    printf("Content-length: %d%s", (int)strlen(content), ENDLINE);
    printf("Content-type: text/html%s%s",ENDLINE,ENDLINE);
    printf("%s", content);
    fflush(stdout);

    exit(0);
}

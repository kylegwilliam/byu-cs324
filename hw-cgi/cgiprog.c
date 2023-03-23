// Im not really sure how to do this. 
// I am just looking at examples from past code to try and figure it out.

#include "csapp.h"
void *thread(void *vargp);

int main()
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = Open_listendfd(argv[1]);
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(linstenfd, 
                (SA *) &clientaddr, &clientlen);
        Pthred_create(&tid, NULL, thread, connfdp);

    }
}

void *thread(void *vargp)
{
    int connfd = *((int*) vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}


#include <stdio.h>
#include"csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct {
    char name[MAXLINE];
    char value[MAXLINE];
}Header;

typedef struct{
    char* name;
    char* object;
}cacheline;

typedef struct{
    int cnt;
    cacheline* objects;
}Cache;

Cache cache;
int count;
sem_t mutex,w;

void doit(int fd);
void parse_uri(int fd,char* host,char* path,char* port,Header* headers,int* num);
int send_to_server(char* host,char* path,char* port,Header* headers,int num);
void* thread(void* v);
void init_cache();
int reader(int fd,char* uri);
void writer(char* uri,char* buf);

void sigpipe_handler(int sig){
  printf("sigpipe handled %d\n", sig);
  return;
}

int main(int argc,char **argv){
    int listenfd;
    int* connfd;
    pthread_t tid;
    char hostname[MAXLINE],port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    if(argc!=2){
        fprintf(stderr,"usage:%s<port>\n",argv[0]);
        exit(1);
    }
    init_cache();
    listenfd=Open_listenfd(argv[1]);
    while (1){
        clientlen=sizeof(clientaddr);
        connfd=Malloc(sizeof(int));
        *connfd=Accept(listenfd,(SA*)&clientaddr,&clientlen);
        Getnameinfo((SA*)&clientaddr,clientlen,hostname,MAXLINE,port,MAXLINE,0);
        printf("Accepted connection from (%s,%s)\n",hostname,port);
        Pthread_create(&tid,NULL,thread,connfd);
    }
    return 0;
}

void doit(int fd){
    char buf[MAXLINE],uri[MAXLINE];
    char host[MAXLINE],path[MAXLINE],port[MAXLINE],buf1[MAX_OBJECT_SIZE];
    rio_t rio;
    int num,size,connfd,n;
    Header headers[20];
    parse_uri(fd,host,path,port,headers,&num);
    strcpy(uri,host);
    strcpy(uri+strlen(uri),path);
    if(reader(fd,uri)){
        printf("%s from cache\n",uri);
        return;
    }
    size=0;
    connfd=send_to_server(host,path,port,headers,num);
    Rio_readinitb(&rio,connfd);
    while((n=Rio_readlineb(&rio,buf,MAXLINE))!=0){
        Rio_writen(fd,buf,n);
        strcpy(buf1+size,buf);
        size+=n;
    }
    if(size<MAX_OBJECT_SIZE) writer(uri,buf1);
    close(connfd);
}

void parse_uri(int fd,char* host,char* path,char* port,Header* headers,int* num){
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    printf("Request headers\n");
    printf("%s",buf);
    sscanf(buf,"%s %s %s",method,uri,version);
    if(strcasecmp(method,"GET")){
        printf("Tiny does not implement this method");
        return;
    }
    if(strstr(uri,"http://")!=uri){
        printf("Error,invalid uri\n");
        return;
    }
    char* temp=uri+strlen("http://");
    char* t=strstr(temp,":");
    *t='\0';
    strcpy(host,temp);
    temp=t+1;
    t=strstr(temp,"/");
    *t='\0';
    strcpy(port,temp);
    *t='/';
    strcpy(path,t);
    *num=0;
    Rio_readlineb(&rio,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        Header h;
        t=strstr(buf,":");
        if(t==NULL){
            printf("Error,invalid header\n");
            return;
        }
        *t='\0';
        strcpy(h.name,buf);
        strcpy(h.value,t+2);
        headers[*num]=h;
        (*num)++;
        Rio_readlineb(&rio,buf,MAXLINE);
    }
}

int send_to_server(char* host,char* path,char* port,Header* headers,int num){
    int fd;
    char buf[MAXLINE];
    char* b=buf;
    rio_t rio;
    fd=Open_clientfd(host,port);
    Rio_readinitb(&rio,fd);
    sprintf(b,"GET %s HTTP/1.0\r\n",path);
    b=b+strlen(buf);
    for(int i=0;i<num;i++){
        sprintf(b,"%s %s",headers[i].name,headers[i].value);
        b=buf+strlen(buf);
    }
    sprintf(b,"\r\n");
    Rio_writen(fd,buf,MAXLINE);
    return fd;
}

void* thread(void* v){
    int fd=*((int*)v);
    Pthread_detach(pthread_self());
    Signal(SIGPIPE,sigpipe_handler);
    Free(v);
    doit(fd);
    Close(fd);
    return NULL;
}

void init_cache() {
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    count = 0;
    cache.objects = (cacheline*)Malloc(sizeof(cacheline) * 10);
    cache.cnt = 0;
    for (int i = 0; i < 10; ++i) {
        cache.objects[i].name = Malloc(sizeof(char) * MAXLINE);
        cache.objects[i].object = Malloc(sizeof(char) * MAX_OBJECT_SIZE);
    }
}

int reader(int fd, char *uri) {
    int in_cache= 0;
    P(&mutex);
    count++;
    if (count == 1) {
        P(&w);
    }
    V(&mutex);

    for (int i = 0; i < 10; ++i) {
        if (!strcmp(cache.objects[i].name, uri)) {
            Rio_writen(fd, cache.objects[i].object, MAX_OBJECT_SIZE);
            in_cache = 1;
            break;
        }
    }
    P(&mutex);
    count--;
    if (count == 0) {
        V(&w);
    }
    V(&mutex);
    return in_cache;
}

void writer(char *uri, char *buf) {
    P(&w);
    strcpy(cache.objects[cache.cnt].name, uri);
    strcpy(cache.objects[cache.cnt].object, buf);
    ++cache.cnt;
    V(&w);
}
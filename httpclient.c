/*******   http客户端程序   httpclient.c   ************/
#include   <stdio.h> 
#include   <stdlib.h> 
#include   <string.h> 
#include   <sys/types.h> 
#include   <sys/socket.h> 
#include   <errno.h> 
#include   <unistd.h> 
#include   <netinet/in.h> 
#include   <limits.h> 
#include   <netdb.h> 
#include   <arpa/inet.h> 
#include   <ctype.h> 
 
//////////////////////////////httpclient.c   开始/////////////////////////////////////////// 
 
/******************************************** 
功能：搜索字符串右边起的第一个匹配字符 
********************************************/
char* Rstrchr(char* s, char x)    
{ 
    int i = strlen(s); 
    if(!(*s)) 
    {
        return 0; 
    }
    while(s[i-1])  
    {
        if(strchr(s+(i-1), x))     
        {
            return (s+(i-1));    
        }
        else  
        {
            i--;
        }
    }
    return 0; 
} 
 
/******************************************** 
功能：把字符串转换为全小写 
********************************************/
void   ToLowerCase(char* s)    
{ 
    while(*s && *s!='\0' )    
    {
        *s=tolower(*s++); 
    }
    *s = '\0';
} 
 
/************************************************************** 
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件 
***************************************************************/
void GetHost(const char* src, char web[], char* file, int* port)    
{ 
    char* pA; 
    char* pB;
    //printf("%d",sizeof(web));
    //初始化
    memset(web, 0, sizeof(web)); 
    memset(file, 0, sizeof(file)); 
    *port = 0;
    //如果地址为空，就返回。。  不过这个应该在argc中判断
    if(!(*src))  
    {
        return; 
    }
    //
    pA = src; 
    //如果src中带有前缀的http:// ，则把前缀略过
    if(!strncmp(pA, "http://", strlen("http://")))   
    {
        pA = src+strlen("http://"); 
    }
    //如果带有前缀的https://,同上
    else if(!strncmp(pA, "https://", strlen( "https://")))     
    {
        pA = src+strlen( "https://"); 
    }
    //pB将指向pA中第一个'/'，用来获取ip或者ip:port
    pB = strchr(pA, '/');
    /* printf("%c" , *pB); */
    //如果路径中存在'/'
    if(pB)     
    {
        //获取ip或ip:port
        memcpy(web, pA, strlen(pA)-strlen(pB)); 
        
        /* printf("---%d" , web[strlen(pA)-strlen(pB)-1]); */
        web[strlen(pA) - strlen(pB)] = '\0'; 
        
    /* printf("----%d" , strlen(web)); */
        // 获取资源的相对地址
        if(*(pB+1))   
        { 
            memcpy(file, pB+1, strlen(pB)-1); 
            file[strlen(pB)-1] = 0; 
        } 
    }
    //如果不存在'/' 则说明域名
    else    
    {
        memcpy(web, pA, strlen(pA)); 
       // printf("---%d" , web[strlen(pA)-1]); 
        web[strlen(pA)] = '\0'; 
    }

    /* printf("%d" , strlen(web)); */
    /* printf("%d" , strlen(pA)); */
    //
    pB = strchr(web, ':'); 
    /* printf("%d" , strlen(pB)); */
    if(pB)    
    {
        /* printf("\n---%c\n" , web[strlen(web)-strlen(pB)]); */
        memcpy(web , web , strlen(web) - strlen(pB)-1);
        /* printf("\n---%d\n" , strlen(pA)); */
        /* printf("\n---%d\n" , strlen(pB)); */
        /* printf("\n---%d\n" , strlen(web)); */

        web[strlen(web)-strlen(pB)] = '\0'; 
       
       // printf("%s", web[strlen(pA)- strlen(pB)-2]);
        *port = atoi(pB + 1); 
    }
    else  
    {
        *port = 80 ; 
    }
} 
 
/********************************************************************* 
*filename:   httpclient.c 
*purpose:   HTTP协议客户端程序，可以用来下载网页 
*********************************************************************/
int   main(int   argc,   char   *argv[]) 
{ 
    int sockfd = 0; 
    char buffer[1024] = ""; 
    struct sockaddr_in   server_addr; 
    struct hostent   *host; 
    int portnumber = 0;
    int nbytes = 0; 
    char host_addr[256] = ""; 
    char host_file[1024] = ""; 
    FILE *fp; 
    char request[1024] = ""; 
    int send = 0;
    int totalsend = 0; 
    int i = 0; 
    char *pt;
    char * srctest = "192.168.1.4";
    //  判断参数是否合法 
    if(argc!=2) 
    { 
        fprintf(stderr, "Usage:%s   web-address\a\n ",argv[0]); 
        exit(1); 
    } 
    printf( "parameter.1 is: %s\n ", argv[1]); 
    //ToLowerCase(argv[1]);/*将参数转换为全小写*/ 
    //printf( "lowercase   parameter.1   is:   %s\n ",   argv[1]); 
 
    GetHost(argv[1], host_addr, host_file, &portnumber);/*分析网址、端口、文件名等*/
    printf( "webhost:%s\n ", host_addr); 
    printf( "hostfile:%s\n ", host_file); 
    printf( "portnumber:%d\n\n ", portnumber); 

    if((host=gethostbyname(host_addr)) == NULL)/*取得主机IP地址*/
    { 
        fprintf(stderr, "Gethostname   error,  ---  %s\n ",   strerror(errno)); 
        exit(1); 
    } 
 
    /*   客户程序开始建立   sockfd描述符   */
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1)/*建立SOCKET连接*/
    { 
        fprintf(stderr, "Socket   Error:%s\a\n ",strerror(errno)); 
        exit(1); 
    } 
 
    /*   客户程序填充服务端的资料   */
    bzero(&server_addr,sizeof(server_addr)); 
    server_addr.sin_family=AF_INET; 
    server_addr.sin_port=htons(portnumber); 
    server_addr.sin_addr=*((struct in_addr*)host->h_addr); 
 
    /*   客户程序发起连接请求   */
    if(connect(sockfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) == -1)/*连接网站*/
    { 
        fprintf(stderr, "Connect   Error:%s\a\n ",strerror(errno)); 
        exit(1); 
    } 
 
    sprintf(request,   "GET   /%s   HTTP/1.1\r\nAccept:   */*\r\nAccept-Language:   zh-cn\r\n\ 
User-Agent:   Mozilla/4.0   (compatible;   MSIE   5.01;   Windows   NT   5.0)\r\n\ 
Host:   %s:%d\r\nConnection:   Close\r\n\r\n ", host_file, host_addr, portnumber); 
     
    printf( "%s\n", request);/*准备request，将要发送给主机*/
 
    /*取得真实的文件名*/
    if(host_file && *host_file)     
    {
        pt = Rstrchr(host_file, '/'); 
    }
    else  
    {
        pt = 0; 
    }
 
    /*发送http请求request*/
    send = 0;
    totalsend = 0; 
    nbytes=strlen(request); 
    while(totalsend < nbytes)  
    { 
        send = write(sockfd, request+totalsend, nbytes-totalsend); 
        if(send == -1)     
        {
            printf( "send error!%s\n ", strerror(errno));
            exit(0);
        } 
        totalsend += send; 
        printf("%d bytes send OK!\n ", totalsend); 
    } 
 
    printf( "\nThe   following   is   the   response   header:\n "); 
    i=0; 
    /*   连接成功了，接收http响应，response   */
    while((nbytes=read(sockfd,buffer,1))==1) 
    {
        //4个\r或\n 为分界
        /* if(i < 4) */    
        /* { */ 
        /*     if(buffer[0] == '\r' || buffer[0] == '\n') */     
        /*     { */
        /*         i++; */ 
        /*     } */
        /*     else */  
        /*     { */
        /*         i = 0; */ 
        /*     } */
        //    printf( "%c", buffer[0]);/*把http头信息打印在屏幕上*/ 
        /* } */ 

         printf( "%c", buffer[0]);/*把http头信息打印在屏幕上*/
    } 
//  printf("\n\nresponse = %s\n", buffer);
    /*   结束通讯   */
    close(sockfd); 
    exit(0); 
} 
//////////////////////////////httpclient.c   结束/////////////////////////////////////////

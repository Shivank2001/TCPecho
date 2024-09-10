#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h> //to create a socket
#include<unistd.h>     //to close the connection, fork
#include<errno.h>      //to check the error number
#include<netinet/in.h>
#define BUFFER_SIZE 20
#define PORT 8080

ssize_t bytesread;
ssize_t byteswritten;




void error_sys();
ssize_t writen(int socket1);

char bufferi[BUFFER_SIZE];
char buffero[BUFFER_SIZE];
 
int main(){
    int socket1;
    struct sockaddr_in serveraddr;
    socket1= socket(AF_INET,SOCK_STREAM,0);//AF_INET implies IPV4 protocol, SOCK_STREAM implies TCP connection, 0 implies Internet Protocol

    if(socket1<0){
        error_sys("Socket creation failed");
    }
    else{
        printf("Socket created");
    }
    
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serveraddr.sin_port=htons(PORT);

    if(connect(socket1,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0){
        error_sys("Connection failed");
    }
    
    writen(socket1);
    printf("Number of bytes written are:%zd\n",byteswritten);
}

ssize_t writen(int socket1){
    for(;;){
        transmit:
            bzero(bufferi, sizeof(bufferi));
            if(fgets(bufferi,sizeof(bufferi),stdin)){
            byteswritten=write(socket1,bufferi,strlen(bufferi));
            if(byteswritten<0){
                if(errno==EINTR){
                    goto transmit;
                }else{
                    return -1;
                }
            }else if(byteswritten==0){
                return -1;
            }
            return byteswritten;
            
            }
    
        
    }
    

    }

void readline(int socket1){
            bytesread= read(socket1,buffero,sizeof(buffero));
            buffero[bytesread] = '\0';
                
            if (bytesread<0 && errno ==EINTR)
            {
                
            }
            fputs(buffero,stdout);
}

void error_sys(const char *message){
    perror(message);
    exit(-1);
}



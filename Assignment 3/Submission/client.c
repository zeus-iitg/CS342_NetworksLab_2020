#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[2049];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    bzero(buffer,2049);
    read(sockfd,buffer,2048);
    printf("%s",buffer);
    scanf("%[^\n]%*c",buffer);

    write(sockfd,buffer,strlen(buffer));
    
    bzero(buffer,2049);
    read(sockfd,buffer,2048);
    printf("%s",buffer);
    
    scanf("%[^\n]%*c",buffer);
    
    write(sockfd,buffer,strlen(buffer));

    bzero(buffer,2049);
    read(sockfd,buffer,2048);
    printf("%s",buffer);

    if(strncmp("Authentication successful\n", buffer, 26))
    {
        close(sockfd);
        return 0;
    }

    else{
        while(1)
        {
            bzero(buffer,2049);
            fgets(buffer,2048,stdin);
            n = write(sockfd,buffer,strlen(buffer));
            if (n < 0) 
                 error("ERROR writing to socket");
            bzero(buffer,2049);
            n = read(sockfd,buffer,2048);
            if (n < 0) 
                 error("ERROR reading from socket");
            printf("%s",buffer);
            int i = strncmp("Connection closed\n" , buffer , 18);
            if(i == 0)
                   break;
        }
    }

    close(sockfd);
    return 0;
}

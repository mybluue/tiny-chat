#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<string>
#include<iostream>

#define BUFFER_LENGTH 4096

int main(int argc,char *argv[])
{
    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(struct sockaddr));
    servAddr.sin_family=PF_INET;
    servAddr.sin_addr.s_addr=inet_addr(argv[1]);
    servAddr.sin_port=htons(atoi(argv[2]));
    socklen_t servLen=sizeof(servAddr);
    int clientSocket=socket(PF_INET,SOCK_DGRAM,0);

    char send_buf[BUFFER_LENGTH];
    char recv_buf[BUFFER_LENGTH];
    std::string head,content,sendmessage,recvmessage;
    
    while(true)
    {
        memset(&send_buf,0,sizeof(send_buf));
        head.clear();
        content.clear();
        sendmessage.clear();
        getline(std::cin,head);
        getline(std::cin,content);

        sendmessage=head+"\n"+content;
        strcpy(send_buf,sendmessage.c_str());
        sendto(clientSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&servAddr,servLen);

        
        // 接收服务器消息并print
        memset(&recv_buf,0,sizeof(recv_buf));
        recvmessage.clear();
        struct sockaddr_in fromAddr;
        socklen_t fromLen=sizeof(fromAddr);
        recvfrom(clientSocket,recv_buf,sizeof(recv_buf),0,(struct sockaddr*)&fromAddr,&fromLen);
        recvmessage=std::string(recv_buf);
        std::cout<<recvmessage<<std::endl;
    }
    
    return 0;
}

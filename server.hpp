#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<string>
#include<vector>
#include<unordered_map>
#include<error.h>
#include<iostream>
#include<sstream>
#include"user.hpp"

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#define BUFFER_LENGTH 4096
#define DEFAULT_PORT 5055

class server
{
public:
    // // 禁止拷贝构造函数和赋值构造函数
    // server(const server &) =delete;
    // server& operator=(const server &) =delete;

    bool setServerSocket();
    bool checkTxt();
    void work();
    void sendMessage(std::string message,struct sockaddr_in dest);
    void sendOnlineList();
    bool checkUserNameAndPassword(std::string username,std::string password);
    bool checkDuplicateLogin(std::string username);
    bool checkDuplicateRigister(std::string username);
    std::string getUsername(std::string ip,std::string port);
    int getUserindex(std::string username);
    void extractLoginUserInfo(std::string userInfo,std::string &username,std::string &password,std::string &receiverport);
    void extractRegisterUserInfo(std::string userInfo,std::string &username,std::string &password);
    void extractPersonalReceivername(std::string &message,std::string &receivername);
private:
    std::string servIP,servPORT;
    int serverSocket;
    struct sockaddr_in serAddr;
    struct sockaddr_in cliAddr;
    int cli_len=sizeof(cliAddr);
    char recv_buf[BUFFER_LENGTH];
    char send_buf[BUFFER_LENGTH];
    std::vector<user> usertable;
    std::unordered_map<std::string,std::string> username2password;
    std::unordered_map<std::string,struct sockaddr_in> username2sock;
    std::unordered_map<std::string,bool> username2online;
    std::string sendmessage,printmessage;   // 存储服务器转发、打印用的字符串
    // int send_len,recv_len;                  // 存储服务器发送和接收的字符串的长度
};

// 使用宏定义的默认端口，bind失败说明默认端口被占用，随机分配其它可用端口
bool server::setServerSocket()
{
    serverSocket=socket(PF_INET,SOCK_DGRAM,0);
    if(serverSocket==-1)
    {
        perror("socket error!\n");
        return false;
    }

    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;
    serAddr.sin_port=htons(DEFAULT_PORT);
    serAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    int ret=bind(serverSocket,(struct sockaddr *)&serAddr,sizeof(struct sockaddr_in));
    // 默认端口被占用使用随机端口
    if(ret==-1)
    {
        serAddr.sin_port=0;
        ret=bind(serverSocket,(struct sockaddr *)&serAddr,sizeof(struct sockaddr_in));
    }
    if(ret==-1)
    {
        perror("bind error!\n");
        return false;
    }
    // 获取服务器IP和PORT
    struct sockaddr_in serAddrCP;
    memset(&serAddrCP,0,sizeof(struct sockaddr_in));
    unsigned int len=sizeof(serAddrCP);
    ret=getsockname(serverSocket,(struct sockaddr *)&serAddrCP,&len);
    if(ret==-1)
    {
        perror("getsockname error!\n");
        return false;
    }
    servIP=(std::string)inet_ntoa(serAddrCP.sin_addr);
    // servIP=std::to_string(ntohl(serAddrCP.sin_addr.s_addr));
    servPORT=std::to_string(ntohs(serAddrCP.sin_port));
    
    std::cout<<"server socket build sucecss:\n"<<"IP: "<<servIP<<'\t'<<"PORT: "<<servPORT<<std::endl;

    return true;
}

void server::sendMessage(std::string message,struct sockaddr_in dest)
{
    char sendbuf[BUFFER_LENGTH];

    strcpy(sendbuf,message.c_str());
    
    // 发送失败，最多重发10次
    int resend=0;
    while(sendto(serverSocket,sendbuf,sizeof(sendbuf),0,(struct sockaddr *)&dest,sizeof(struct sockaddr_in))==-1 && resend<10){resend++;};
}

void server::work()
{
    std::cout<<"-----------------"<<std::endl;
    std::cout<<"server running..."<<std::endl;
    std::cout<<"-----------------"<<std::endl;
    while(true)
    {
        sendmessage.clear();
        // 初始化接收缓存
        memset(&recv_buf,0,sizeof(recv_buf)); 
        socklen_t cliLen=sizeof(struct sockaddr);
        int irecv=recvfrom(serverSocket,recv_buf,BUFFER_LENGTH,0,(struct sockaddr*)&cliAddr,&cliLen);
        
        if(irecv==-1)
        {
            perror("recvfrom error!\n");
            continue;
        }

        // 抽取请求报文的head和content
        std::string head,content;
        int i=0;
        for(int i=0;i<irecv && recv_buf[i]!='\n';++i)
        {
            head+=recv_buf[i];
        }
        for(;i<irecv;++i)
        {
            content+=recv_buf[i];
        }

        // 抽取head信息
        std::vector<std::string> headInfo;
        std::istringstream t(head);
        std::string s;
        while(t>>s)
        {
            headInfo.push_back(s);
        }

        // 更新username和sockaddr_in的对应关系
        username2sock[headInfo[1]]=cliAddr;

        // 处理请求
        if(headInfo[0]=="logout")
        {
            username2online[headInfo[1]]=false;
        }
        else if(headInfo[0]=="register")
        {
            if(username2password.count(headInfo[1])!=0)
            {
                sendmessage="用户名已被注册，注册失败\n";
                strcpy(send_buf,sendmessage.c_str());
                sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&cliAddr,cliLen);
            }
            else
            {
                sendmessage="您已注册成功：\n用户名： "+headInfo[1]+'\n'+"密码： "+headInfo[2]+'\n';
                strcpy(send_buf,sendmessage.c_str());
                sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&cliAddr,cliLen);
                username2password[headInfo[1]]=username2password[headInfo[2]];
            }
        }
        else if(headInfo[0]=="login")
        {
            if(username2password.count(headInfo[1])==0 || username2password[headInfo[1]]!=username2password[headInfo[2]])
            {
                sendmessage="用户名或密码有误，登录失败！\n";
                strcpy(send_buf,sendmessage.c_str());
                sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&cliAddr,cliLen);
            }
            else
            {
                sendmessage="登录成功,欢迎 "+headInfo[1]+'\n';
                strcpy(send_buf,sendmessage.c_str());
                sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&cliAddr,cliLen);
                username2online[headInfo[1]]=true;
            }
        }
        else if(headInfo[0]=="personal")
        {
            sendmessage=headInfo[1]+": "+content+'\n';
            strcpy(send_buf,sendmessage.c_str());
            struct sockaddr_in destAddr=username2sock[headInfo[2]];
            sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&destAddr,cliLen); 
        }

        else
        {
            sendmessage="请求错误！\n";
            strcpy(send_buf,sendmessage.c_str());
            sendto(serverSocket,send_buf,sizeof(send_buf),0,(struct sockaddr *)&cliAddr,cliLen);
        }

    }

}


#endif

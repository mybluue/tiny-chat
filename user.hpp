#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<string>

#ifndef __USER_HPP__
#define __USER_HPP__

class user
{
public:
    user(std::string username,std::string ip,int sender_port,int receiver_port)
    {
        this->username=username;
        this->ip=ip;
        this->sender_port=sender_port;
        this->receiver_port=receiver_port;
        // 配置receiver地址
        memset(&receiver,0,sizeof(receiver));
        receiver.sin_family=AF_INET;
        receiver.sin_port=htons(receiver_port);
        receiver.sin_addr.s_addr=inet_addr(ip.c_str());

    }

    std::string username;           // 用户名
    std::string ip;                 // 客户端IP地址
    int sender_port;                // 发送器端口
    int receiver_port;              // 接收器端口
    struct sockaddr_in receiver;    // 存储接收器的地址
};

#endif
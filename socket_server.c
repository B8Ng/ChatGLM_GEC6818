#include "socket_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
int Server_Init(int Port, char *IPAddress)
{
    /*创建客户端对象(不参与传输)*/
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0)
    {
        printf("创建socket失败\n");
        return -1;
    }
    else
    {
        printf("创建socket成功\n");
    }
    /*设置服务器地址信息*/
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;                   // 设置网络层协议
    addr.sin_port = htons(Port);                 // 设置端口号
    addr.sin_addr.s_addr = inet_addr(IPAddress); // 设置IP地址
    /*绑定服务器地址信息*/
    if (bind(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("绑定失败\n");
        return -1;
    }
    else
    {
        printf("绑定成功\n");
    }
    /*设置成监听模式*/
    if (listen(tcp_socket, 2) < 0)
    {
        printf("监听失败\n");
        return -1;
    }
    else
    {
        printf("监听成功\n");
    }

    return tcp_socket;
}

int Server_Accept(int tcp_socket)
{
    printf("等待客户端连接........\n");
    int new_socket = accept(tcp_socket, NULL, NULL);
    printf("客户端连接成功 %d\n", new_socket);
    return new_socket;
}


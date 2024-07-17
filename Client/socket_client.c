#include "socket_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
struct sockaddr_in addr;
int Client_Init(int Port,char *IPAddress)
{
    /*创建客户端对象*/
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0)
    {
        printf("创建socket失败\n");
        return 0;
    }
    else
    {
        printf("创建socket成功\n");
    }

    /*设置服务器地址信息*/
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Port);                        // 端口
    addr.sin_addr.s_addr = inet_addr(IPAddress);

    return tcp_socket;
}

void Connect_Client(int tcp_socket)
{
    while (1)
    {
        /*连接服务器*/
        int ret = connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0)
        {
            printf("连接服务器失败\n");
        }
        else
        {
            printf("连接服务器成功\n");
            break;
        }
    }
}
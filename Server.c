
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "iat.h"
#include "socket_server.h"

int main()
{
   int tcp_socket = Server_Init(1234,"192.168.46.103");//socket初始化

label:
    int new_socket = Server_Accept(tcp_socket);//初始化为监听模式

    while (1)
    {
        /*接收任务*/
        char recv_buf[2048] = {0};//接收数据缓存区
        int recvsize = read(new_socket, recv_buf, sizeof(recv_buf));
        printf("接收中\n");
		printf("包大小:%d\n",recvsize);
        if(recvsize <= 0) //若接收失败重新等待客户端连接服务器
        {	
        	printf("客户端已断开！\n");
        	close(new_socket);
        	goto label;
        }

        //任务一：语音识别
        if(strcmp(recv_buf,"Detect")==0)
        {
            int fp0 = open("./Client/test.wav",O_RDWR|O_CREAT|O_TRUNC,0777);//创建test.wav文件(可读写，创建标识符，权限777)
            if(fp0 <0)
            {
                printf("创建失败\n");
                return -1;
            }
            else
            {
                printf("创建成功\n");
            }
            
            int recvsizemax = 0;
            while (1)
            {
                char recv_buf[2048] = {0};//接收数据缓存区
                int recvsize = read(new_socket, recv_buf, sizeof(recv_buf));//接收6818数据到PC
                printf("接收中\n");
                printf("包大小:%d\n",recvsize);
                if(recvsize <= 0) //若接收失败重新等待客户端连接服务器
                {	
                    printf("客户端已断开！\n");
                    close(new_socket);
                    goto label;
                }
                recvsizemax = recvsizemax+recvsize;
                printf("获取包总大小:%d\n",recvsizemax);
                write(fp0,recv_buf,sizeof(recv_buf));//写入数据到test.wav文件
                printf("写入wav一次:%d\n",recvsize);
                if (recvsizemax >= 96888)
                {
                    printf("接收完成\n");
                    system("ls -ls ./Client/test.wav");
                    close(fp0);
                    break;//退出接收数据循环
                }
                
            }
            char buf[1024] = {0};//数据缓存区
            wav_to_str(buf);//音频转换str
            write(new_socket,buf,sizeof(buf));//发送数据到客户端
            printf("发送问题回客户端\n");
            
        }

        //任务二：AI问答
        else if (strcmp(recv_buf,"Ask")==0)
        {
            write(new_socket,"ok",2);//返回确认任务执行信号到客户端
            /*服务器接收问题*/
            while(1)
            {
                char recv_buf[2048] = {0};//接收数据缓存区
                int recvsize = read(new_socket, recv_buf, sizeof(recv_buf));//接收6818数据到PC
                printf("接收中\n");
                printf("包大小:%d\n",recvsize);
                if(recvsize <= 0) //若接收失败重新等待客户端连接服务器
                {	
                    printf("客户端已断开！\n");
                    close(new_socket);//断开连接
                    goto label;
                }
                printf("接收到的问题 :%s\n",recv_buf);
                char buf[4096]={0};//AI回答数据缓存区
                char top[40]="python3 Client/AI.py ";//python指令
                strcat(top,recv_buf);//拼接指令
                /*加载python脚本*/
                FILE *fp1 = popen(top, "r");
                if (fp1 == NULL)
                {
                    printf("调用API失败！\n");
                    return -1;
                }
                /*读取脚本返回的数据循环*/
                while (1)
                {
                    // 读取脚本返回的数据 到 buf 中
                    char *ret = fgets(buf, sizeof(buf), fp1);
                    if (ret == NULL)
                    {
                        printf("调用API成功！\n");
                        break;
                    }
                }
                printf("————————————AI回答————————————\n");
                printf("%s\n",buf);//打印AI返回的答案
                pclose(fp1);//关闭文件流
                int writesize = write(new_socket, buf, strlen(buf));//发送答案到客户端
                printf("发送答案大小：%d\n",writesize);
                break;
            }
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include "lcd.h"
#include "font.h"
#include "history.h"
#include "unistd.h"
#include "socket_client.h"
int main()
{
    CircularBuffer cbuf;
    initBuffer(&cbuf);                                    // 初始化历史记录
    int tcp_socket = Client_Init(1234, "192.168.46.103"); // socket初始化,连接服务器
    Connect_Client(tcp_socket);                           // 连接服务器
    char question[1024] = {0};                            // 问题内容缓存区
    char answer[4096] = {0};                              // 答案内容缓存区
    int x = 0, y = 0;                                     // 初始化触控点坐标
    int xb = 0, yb = 0;                                   // 初始化第二次触控点坐标
    int add_y = 32;                                       // 滑动一次距离
    int default_y = 0;
    char answer_detect[4096] = {0};
    Show_Main_Init(); // 主页GUI
    /*任务主循环*/
    while (1)
    {
        x = 0;
        y = 0; // 刷新触控点
        xb = 0;
        yb = 0;


        get_touch(&x, &y, &xb, &yb);

        printf("我出来了！\n");

        if (x == -10 && y == -20) // 上
        {
            if (default_y + add_y <= 9 * 32)
            {
                default_y = default_y + add_y;
                printf("default_y = %d\n", default_y);
                printf("答案存储为：%s\n", answer_detect);
                if ((getLastQA(&cbuf) && getLastQA(&cbuf)->answer && strlen(getLastQA(&cbuf)->answer)) > 0)
                {
                    if (strcmp(answer_detect, "输入为空,会话退出") == 10)
                    {
                        printf("滑动成功！\n");
                    }
                    else
                    {
                        Update_Response(getLastQA(&cbuf)->answer, -default_y); // 最后一次回答的buffer
                        printf("显示成功！\n");
                    }
                }
                else
                {
                    printf("滑动成功！\n");
                    printf("没有东西更新\n");
                }
            }
        }
        else if (x == -20 && y == -10) // 下
        {
            if (default_y - add_y >= 0)
            {
                default_y = default_y - add_y;
                printf("default_y = %d\n", default_y);
                printf("答案存储为：%s\n", answer_detect);
                printf("比对结果：%d\n", strcmp(answer_detect, "输入为空,会话退出"));
                if ((getLastQA(&cbuf) && getLastQA(&cbuf)->answer && strlen(getLastQA(&cbuf)->answer)) > 0)
                {
                    if (strcmp(answer_detect, "输入为空,会话退出") == 10)
                    {
                        printf("滑动成功！\n");
                    }
                    else
                    {
                        Update_Response(getLastQA(&cbuf)->answer, -default_y); // 最后一次回答的buffer
                        printf("显示成功！\n");
                    }
                }
                else
                {
                    printf("滑动成功！\n");
                    printf("没有东西更新\n");
                }
            }
        }
        /*语音输入按钮*/
        if (x > 8 && x < 258 && y > 390 && y < 470)
        {
            Update_Qustion("正在进行语音识别……\n");
            // 语音识别代码

            write(tcp_socket, "Detect", 6);                                 // 发送指令到服务器
            system("arecord -d 3 -r 16000 -c 1 -f S16_LE -t wav test.wav"); // 发送录音指令，生成test.wav

            int fp = open("test.wav", O_RDONLY); // 打开test.wav文件(只读)
            system("ls -ls test.wav");
            if (fp < 0) // 若为空关闭客户端连接
            {
                printf("打不开文件\n");
                close(tcp_socket);
                return 0;
            }
            char sendbuf[4096] = {0};   // 发送数据缓冲区
            char recv_buf[4096] = {0};  // 接收数据缓存区
            ssize_t readsize, sendsize; // 接收和发送数据大小
            while (1)
            {
                while ((readsize = read(fp, sendbuf, sizeof(sendbuf))) > 0) // 若读取fp内容不为0则继续发送数据
                {
                    sendsize = write(tcp_socket, sendbuf, readsize); // 发送wav数据到PC
                    printf("发送了一次大小：%ld\n", sendsize);
                    printf("发送成功\n"); // 发送部分数据
                    if (sendsize < 0)     // 若发送失败
                    {
                        printf("发送失败\n");
                        close(tcp_socket);
                        close(fp); // 关闭fp
                        Connect_Client(tcp_socket);
                    }
                }

                printf("发送全部数据完成\n");
                close(fp); // 关闭fp
                break;     // 退出发送数据循环
            }
            read(tcp_socket, recv_buf, sizeof(recv_buf)); // 接收服务器数据到6818
            printf("接收的答案为：%s\n", recv_buf);
            strcpy(question, recv_buf); // 将接收的问题内容保存到缓存区

            if (strcmp(question, "") != 0)
            {
                Update_Qustion(question); // 显示问题到LCD
            }
            else if (strcmp(question, "") == 0)
            {
                Update_Qustion("未识别到语音，请重新输入！");
            }
        }
        /*AI问答按钮*/
        else if (x > 274 && x < 524 && y > 390 && y < 470)
        {
            printf("语义理解按键按下\n");
            Update_Response("正在进行语义理解……\n", 0);
            // 调用ChatGLM模型

            char buf[1024] = {0}; // 接收缓存区

            /*请求服务器任务*/
            write(tcp_socket, "Ask", 3);
            while (1)
            {
                read(tcp_socket, buf, 2);
                if (strcmp(buf, "ok") == 0)
                {
                    break;
                }
            }

            write(tcp_socket, question, sizeof(question)); // 接收服务器数据到6818
            printf("发送的问题为：%s\n", question);
            read(tcp_socket, answer, sizeof(answer)); // 接收服务器数据到6818
            printf("接收的答案为：%s\n", answer);

            if (strcmp(question, "") != 0)
            {
                addQA(&cbuf, question, answer);
                Update_Response(answer, 0);
            }
            else if (strcmp(question, "") == 0)
            {
                Update_Response("请重新录入声音！", 0);
            }
            strcpy(answer_detect, answer);
            memset(answer, 0, 4096); // 清空answer缓存区
        }
        /*历史记录按钮*/
        else if (x > 540 && x < 793 && y > 390 && y < 470)
        {
            add_y = 32; // 滑动一次距离
            default_y = 0;
            printf("历史记录按键按下\n");
            Show_History_Init(); // 初始化历史记录界面
            printf("%d\n", cbuf.count);
            if (cbuf.count == 1) // 历史记录最大为四条
            {
                Update_Question1_For_History(cbuf.buffers[0].question);
                Update_Text_For_History(cbuf.buffers[0].answer, 0);
            }
            else if (cbuf.count == 2)
            {
                Update_Question1_For_History(cbuf.buffers[0].question);
                Update_Question2_For_History(cbuf.buffers[1].question);
                Update_Text_For_History(cbuf.buffers[1].answer, 0);
            }
            else if (cbuf.count == 3)
            {
                Update_Question1_For_History(cbuf.buffers[0].question);
                Update_Question2_For_History(cbuf.buffers[1].question);
                Update_Question3_For_History(cbuf.buffers[2].question);
                Update_Text_For_History(cbuf.buffers[1].answer, 0);
            }
            else if (cbuf.count == 4)
            {
                Update_Question1_For_History(cbuf.buffers[0].question);
                Update_Question2_For_History(cbuf.buffers[1].question);
                Update_Question3_For_History(cbuf.buffers[2].question);
                Update_Question4_For_History(cbuf.buffers[3].question);
                Update_Text_For_History(cbuf.buffers[1].answer, 0);
            }

            // 这里应该把历史记录1答案文本、历史记录所有问题载入
            while (1)
            {
                x = 0;
                y = 0; // 刷新触控点
                get_touch(&x, &y, &xb, &yb);
                // 返回键
                if (x > 8 && x < 258 && y > 357 && y < 427)
                {
                    printf("返回主界面按键按下\n");
                    Show_Main_Init(); // 再次初始化
                    if (getLastQA(&cbuf)->question != NULL)
                    {
                        Update_Qustion(getLastQA(&cbuf)->question);   // 应该更新最后一次问题的buffer
                        Update_Response(getLastQA(&cbuf)->answer, 0); // 最后一次回答的buffer
                    }
                    memset(answer_detect, 0, 4096);
                    break;
                }
                // 历史问题1
                else if (x > 8 && x < 258 && y > 8 && y < 78 && cbuf.count >= 1)
                {
                    printf("历史问题1按键按下\n");
                    Update_Text_For_History(cbuf.buffers[0].answer, 0);
                    quecnt = 1;
                }
                // 历史问题2
                else if (x > 8 && x < 258 && y > 94 && y < 164 && cbuf.count >= 2)
                {
                    printf("历史问题2按键按下\n");
                    Update_Text_For_History(cbuf.buffers[1].answer, 0);
                    quecnt = 2;
                }
                // 历史问题3
                else if (x > 8 && x < 258 && y > 180 && y < 250 && cbuf.count >= 3)
                {
                    printf("历史问题3按键按下\n");
                    Update_Text_For_History(cbuf.buffers[2].answer, 0);
                    quecnt = 3;
                }
                else if (x > 8 && x < 258 && y > 266 && y < 336 && cbuf.count >= 4)
                {
                    printf("历史问题4按键按下\n");
                    Update_Text_For_History(cbuf.buffers[3].answer, 0);
                    quecnt = 4;
                }
                else if (x == -10 && y == -20) // 上
                {
                    printf("X:%d Y:%d\n", x, y);
                    switch (quecnt)
                    {
                    case 1:
                        if (default_y + add_y <= 15 * 32)
                        {
                            default_y = default_y + add_y;
                            Update_Text_For_History(cbuf.buffers[0].answer, -default_y);
                            break;
                        }
                    case 2:
                        if (default_y + add_y <= 15 * 32)
                        {
                            default_y = default_y + add_y;
                            Update_Text_For_History(cbuf.buffers[1].answer, -default_y);
                            break;
                        }
                    case 3:
                        if (default_y + add_y <= 15 * 32)
                        {
                            default_y = default_y + add_y;
                            Update_Text_For_History(cbuf.buffers[2].answer, -default_y);
                            break;
                        }
                    case 4:
                        if (default_y + add_y <= 15 * 32)
                        {
                            default_y = default_y + add_y;
                            Update_Text_For_History(cbuf.buffers[3].answer, -default_y);
                            break;
                        }
                    }
                }
                else if (x == -20 && y == -10) // 下
                {
                    switch (quecnt)
                    {
                    case 1:
                        if (default_y + add_y >= 64)
                        {
                            default_y = default_y - add_y;
                            Update_Text_For_History(cbuf.buffers[0].answer, -default_y);
                            break;
                        }
                    case 2:
                        if (default_y + add_y >= 64)
                        {
                            default_y = default_y - add_y;
                            Update_Text_For_History(cbuf.buffers[1].answer, -default_y);
                            break;
                        }
                    case 3:
                        if (default_y + add_y >= 64)
                        {
                            default_y = default_y - add_y;
                            Update_Text_For_History(cbuf.buffers[2].answer, -default_y);
                            break;
                        }
                    case 4:
                        if (default_y + add_y >= 64)
                        {
                            default_y = default_y - add_y;
                            Update_Text_For_History(cbuf.buffers[3].answer, -default_y);
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}
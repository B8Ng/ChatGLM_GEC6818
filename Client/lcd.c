#include "lcd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h> 
#include "font.h"
#include <linux/input.h>
#include "history.h"
/*********************触控屏*********************/
// 获取触摸屏坐标:输入x和y的地址
int get_touch(int *x, int *y,int *xb,int *yb)
{

    // 1.打开触摸屏驱动设备文件
    int fd = open("/dev/input/event0", O_RDWR);
    if (fd < 0)
    {
        printf("打开触摸屏设备失败\n");
        return -1;
    }

    // 2.不断读取触摸屏设备的输入信息到模型中
    while (1)
    {
        struct input_event ev;
        read(fd, &ev, sizeof(ev));

        if(EV_ABS == ev.type)//接下来的数据就是坐标
        {
            if(REL_X == ev.code)//x轴
            {
                *x = ev.value * 800 / 1024;
            }
            else if(REL_Y == ev.code)//y轴
            {
                *y = ev.value* 480 / 600;
            }
        }
        if(0x01 == ev.type && BTN_TOUCH == ev.code && 0x01 == ev.value)//手按下去的时候
        {
            *xb = *x;
            *yb = *y;
            printf("手指按下！\n");
        }
        else if(0x01 == ev.type && BTN_TOUCH == ev.code && 0x00 == ev.value)//手抬起来
        {
            //实现点击和方向判断
            if(*xb == *x && *yb == *y)//你的手没有动
            {
                printf("点击\n");
                break;
            }
            else//滑动 滑动就会有方向
            {
                if(*y > *yb && abs(*y -*yb) > abs(*x - *xb))
                {
                    lseek(fd,0x00,SEEK_SET);
                    *x = -10;
                    *y = -20;
                    printf("上滑\n");
                    break;
                }
                else if(*y < *yb && abs(*y -*yb) > abs(*x - *xb))
                {
                    lseek(fd,0x00,SEEK_SET);
                    *x = -20;
                    *y = -10;
                    printf("下滑\n");
                    break;
                }
            }
        }
    }

    printf("x=%d, y=%d\n", *x, *y);

    printf("xb=%d, yb=%d\n", *xb, *yb);
    
    close(fd);
}

/*********************触控屏*********************/

/*********************LCD显示********************/
// 显示色块:屏幕, x的起点,x的大小,y的起点,y的大小,颜色(getColor(A,B,G,R))
void Show_Color(void *p, int x_start, int x_size, int y_start, int y_size, int rgb)
{
    int *lcd = p; // 重映射指针
    for (int y = y_start; y < y_size; y++)
    {
        for (int x = x_start; x < x_size; x++)
        {
            *(lcd + y * x_size + x) = rgb;
        }
    }
}
// 刷全屏: 需要映射的屏幕指针,颜色
void Show_Clear(int *lcd, int color)
{
    for (int y = 0; y < 480; y++)
    {
        for (int x = 0; x < 800; x++)
        {
            *(lcd + y * 800 + x) = color;
        }
    }
}

// 映射LCD设备
void *mmap_lcd()
{ // 1.打开LCD设备
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0)
    {
        // printf("打开LCD设备失败\n");
        return NULL;
    }

    // 2.把硬件设备LCD的地址映射到应用层中
    void *p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
    {
        printf("映射失败\n");
        return NULL;
    }
    else
    {
        // printf("映射成功\n");
        return p;
    }
}

// 初始化字库
font *init_font(int size)
{
    // 3.加载字库文件
    font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
    if (f == NULL)
    {
        printf("加载字库失败\n");
        return NULL;
    }

    // 4.设置字体大小
    fontSetSize(f, size);
}

// 显示汉字:画板宽度,画板高度,画板颜色,字库指针,文本起始位置x,文本起始位置y,需要显示的文本,
// 字体颜色,最大换行像素,需要映射的屏幕指针,屏幕显示位置x,屏幕显示位置y
void show_text(u32 b_w, u32 b_h, u32 b_color,
               font *f, s32 t_x, s32 t_y, char *text, u32 t_color, s32 maxWidth,
               unsigned int *lcd, int px, int py)
{
    // 创建一个字体输出框  ,并设置为绿色
    bitmap *bm = createBitmapWithInit(b_w, b_h, 4, b_color);

    // 放入文件到画框中
    fontPrint(f, bm, t_x, t_y, text, t_color, maxWidth);

    show_font_to_lcd(lcd, px, py, bm);

    // 销毁画板
    destroyBitmap(bm);
}

// --------------------------------显示部分----------------------------

// 主菜单初始化
void Show_Main_Init()
{
    int *lcd = mmap_lcd();

    Show_Clear(lcd, getColor(255, 255, 255, 255));

    font *f = init_font(64); // 按钮 问题文本字号为64
    show_text(785, 70, getColor(0, 255, 255, 0), f, 0, 0, "请输入问题……", getColor(0, 255, 0, 0), 800, lcd, 8, 8);
    // 三个按钮
    show_text(250, 80, getColor(0, 255, 255, 0), f, 23, 10, "语音输入", getColor(0, 255, 0, 0), 256, lcd, 8, 390);
    show_text(250, 80, getColor(0, 255, 255, 0), f, 23, 10, "语义理解", getColor(0, 255, 0, 0), 256, lcd, 274, 390);
    show_text(253, 80, getColor(0, 255, 255, 0), f, 23, 10, "历史记录", getColor(0, 255, 0, 0), 256, lcd, 540, 390);

    font *text = init_font(32); // 服务器返回文本字号为32

    show_text(785, 290, getColor(0, 255, 255, 0), text, 0, 0, "等待服务器……", getColor(0, 255, 0, 0), 800, lcd, 8, 90);
}
// 主界面部分
// 刷新问题文本:传入需要显示的文本buffer
// 如果换行出现问题修改倒数第四个值就行
void Update_Qustion(char *buffer)
{
    int *lcd = mmap_lcd();
    font *f = init_font(64); // 问题文本字号为64
    show_text(785, 70, getColor(0, 255, 255, 0), f, 0, 0, buffer, getColor(0, 255, 0, 0), 800, lcd, 8, 8);
}

// 刷新回答文本:传入需要显示的系统回复buffer
void Update_Response(char *buffer,int y)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32); // 问题文本字号为32

    show_text(785, 290, getColor(0, 255, 255, 0), text, 0, y, buffer, getColor(0, 255, 0, 0), 780, lcd, 8, 90);
}

// 历史记录界面初始化
void Show_History_Init()
{
    int *lcd = mmap_lcd();

    Show_Clear(lcd, getColor(255, 255, 255, 255));

    font *text = init_font(32);
    // 历史回答框
    show_text(522, 420, getColor(0, 255, 255, 0), text, 0, 0, "暂无历史记录……", getColor(0, 255, 0, 0), 280, lcd, 270, 8);

    show_text(250, 70, getColor(0, 255, 255, 0), text, 20, 15, "暂无历史问题……", getColor(0, 255, 0, 0), 250, lcd, 8, 8);

    font *f = init_font(64); // 按钮大小

    show_text(250, 70, getColor(0, 255, 255, 0), f, 50, 3, "返    回", getColor(0, 255, 0, 0), 200, lcd, 8, 357);
}

// 更新历史问题文本框
void Update_Text_For_History(char *buffer,int y)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32);
    // 历史回答框
    show_text(522, 420, getColor(0, 255, 255, 0), text, 8, y+8, buffer, getColor(0, 255, 0, 0), 508, lcd, 270, 8);
}
// 更新历史问题1
void Update_Question1_For_History(char *buffer)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32);
    show_text(250, 70, getColor(0, 255, 255, 0), text, 10, 10, buffer, getColor(0, 255, 0, 0), 245, lcd, 8, 8);
}

// 更新历史问题2
void Update_Question2_For_History(char *buffer)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32);
    // 历史问题2
    show_text(250, 70, getColor(0, 255, 255, 0), text, 10, 10, buffer, getColor(0, 255, 0, 0), 245, lcd, 8, 94);
}

// 更新历史问题3
void Update_Question3_For_History(char *buffer)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32);
    // 历史问题3
    show_text(250, 70, getColor(0, 255, 255, 0), text, 10, 10, buffer, getColor(0, 255, 0, 0), 245, lcd, 8, 180);
}

void Update_Question4_For_History(char *buffer)
{
    int *lcd = mmap_lcd();
    font *text = init_font(32);
    // 历史问题3
    show_text(250, 70, getColor(0, 255, 255, 0), text, 10, 10, buffer, getColor(0, 255, 0, 0), 245, lcd, 8, 266);
}
/*********************LCD显示********************/
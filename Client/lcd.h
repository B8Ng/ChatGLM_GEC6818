#ifndef __LCD_H__
#define __LCD_H__
#include "font.h"
int get_touch(int *x, int *y, int *xb , int *yb);//x，y第一次坐标检测，xb，yb第二次坐标检测
void Show_Color(void *p, int x_start, int x_size, int y_start, int y_size, int rgb);
void Show_Clear(int *lcd, int color);
void *mmap_lcd();
font *init_font(int size);
void show_text(u32 b_w, u32 b_h, u32 b_color,font *f, s32 t_x, s32 t_y, char *text, u32 t_color, s32 maxWidth,unsigned int *lcd, int px, int py);
void Show_Main_Init();
void Update_Qustion(char *buffer);
void Update_Response(char *buffer,int y);//默认y=0,更改y移动文字
void Show_History_Init();
void Update_Text_For_History(char *buffer,int y);//默认y=0,更改y移动文字
void Update_Question1_For_History(char *buffer);
void Update_Question2_For_History(char *buffer);
void Update_Question3_For_History(char *buffer);
void Update_Question4_For_History(char *buffer);
#endif

#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************** 历史记录 *********************/

// 初始化循环缓冲区
void initBuffer(CircularBuffer *cbuf) {
    cbuf->count = 0;
}

// 添加一个新的问题和答案到循环缓冲区
void addQA(CircularBuffer *cbuf, const char *question, const char *answer) {
    if (cbuf->count < BUFFER_SIZE) {
        strncpy(cbuf->buffers[cbuf->count].question, question, MAX_QUESTION_LENGTH);
        strncpy(cbuf->buffers[cbuf->count].answer, answer, MAX_ANSWER_LENGTH);
        cbuf->count++;
    } else {
        // 将所有现有元素向前移动一个位置
        for (int i = 1; i < BUFFER_SIZE; i++) {
            strncpy(cbuf->buffers[i-1].question, cbuf->buffers[i].question, MAX_QUESTION_LENGTH);
            strncpy(cbuf->buffers[i-1].answer, cbuf->buffers[i].answer, MAX_ANSWER_LENGTH);
        }
        // 将新元素添加到最后一个位置
        strncpy(cbuf->buffers[BUFFER_SIZE-1].question, question, MAX_QUESTION_LENGTH);
        strncpy(cbuf->buffers[BUFFER_SIZE-1].answer, answer, MAX_ANSWER_LENGTH);
    }
}
// 获取最后一个问题和答案的buffer
QABuffer* getLastQA(CircularBuffer *cbuf) {
    if (cbuf->count == 0) {
        return NULL; // 缓冲区为空
    }
    return &cbuf->buffers[cbuf->count - 1];
}
/****************** 历史记录 *********************/

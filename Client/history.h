#ifndef __HISTORY_H__
#define __HISTORY_H__



#define BUFFER_SIZE 4               //历史记录buffer
#define MAX_QUESTION_LENGTH 1024    //历史记录最大问题长度
#define MAX_ANSWER_LENGTH 4096      //历史记录最大回答长度

typedef struct {
    char question[MAX_QUESTION_LENGTH];
    char answer[MAX_ANSWER_LENGTH];
} QABuffer;

typedef struct {
    QABuffer buffers[BUFFER_SIZE];
    int count;
} CircularBuffer;

void initBuffer(CircularBuffer *cbuf);
void addQA(CircularBuffer *cbuf, const char *question, const char *answer);
QABuffer* getLastQA(CircularBuffer *cbuf);

#endif

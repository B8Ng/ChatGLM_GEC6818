#include "iat.h"
#include "./Include/qisr.h"
#include "./Include/msp_cmn.h"
#include "./Include/msp_errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define BUFFER_SIZE 4096
#define FRAME_LEN 640
#define HINTS_SIZE 100

/*iat参数*/
int ret = MSP_SUCCESS;
const char *login_params = "appid = 24fe8644, work_dir = .";
const char *session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8";

// 函数名： run_iat
// 参数：  	audio_file(音频文件地址) session_begin_params(语音识别API参数) result(返回字符串)
// 返回值： 无
// 作用：   讯飞语音识别为字符串
void run_iat(const char *audio_file, const char *session_begin_params, char *result)
{
    const char *session_id = NULL;
    char rec_result[BUFFER_SIZE] = {0};
    char hints[HINTS_SIZE] = {0}; // hints为结束本次会话的原因描述，由用户自定义
    unsigned int total_len = 0;
    int aud_stat = MSP_AUDIO_SAMPLE_CONTINUE; // 音频状态
    int ep_stat = MSP_EP_LOOKING_FOR_SPEECH;  // 端点检测
    int rec_stat = MSP_REC_STATUS_SUCCESS;    // 识别状态
    int errcode = MSP_SUCCESS;

    FILE *f_pcm = NULL;
    char *p_pcm = NULL;
    long pcm_count = 0;
    long pcm_size = 0;
    long read_size = 0;

    if (NULL == audio_file)
        goto iat_exit;

    f_pcm = fopen(audio_file, "rb");
    if (NULL == f_pcm)
    {
        printf("\nopen [%s] failed! \n", audio_file);
        goto iat_exit;
    }

    fseek(f_pcm, 0, SEEK_END);
    pcm_size = ftell(f_pcm); // 获取音频文件大小
    fseek(f_pcm, 0, SEEK_SET);

    p_pcm = (char *)malloc(pcm_size);
    if (NULL == p_pcm)
    {
        printf("\nout of memory! \n");
        goto iat_exit;
    }

    read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); // 读取音频文件内容
    if (read_size != pcm_size)
    {
        printf("\nread [%s] error!\n", audio_file);
        goto iat_exit;
    }

    printf("\n开始语音听写 ...\n");
    session_id = QISRSessionBegin(NULL, session_begin_params, &errcode); // 听写不需要语法，第一个参数为NULL
    if (MSP_SUCCESS != errcode)
    {
        printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
        goto iat_exit;
    }

    while (1)
    {
        unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
        int ret = 0;

        if (pcm_size < 2 * len)
            len = pcm_size;
        if (len <= 0)
            break;

        aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
        if (0 == pcm_count)
            aud_stat = MSP_AUDIO_SAMPLE_FIRST;

        printf(">");
        ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
        if (MSP_SUCCESS != ret)
        {
            printf("\nQISRAudioWrite failed! error code:%d\n", ret);
            goto iat_exit;
        }

        pcm_count += (long)len;
        pcm_size -= (long)len;

        if (MSP_REC_STATUS_SUCCESS == rec_stat) // 已经有部分听写结果
        {
            const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
            if (MSP_SUCCESS != errcode)
            {
                printf("\nQISRGetResult failed! error code: %d\n", errcode);
                goto iat_exit;
            }
            if (NULL != rslt)
            {
                unsigned int rslt_len = strlen(rslt);
                total_len += rslt_len;
                if (total_len >= BUFFER_SIZE)
                {
                    printf("\nno enough buffer for rec_result !\n");
                    goto iat_exit;
                }
                strncat(rec_result, rslt, rslt_len);
            }
        }

        if (MSP_EP_AFTER_SPEECH == ep_stat)
            break;
        usleep(200 * 1000); // 模拟人说话时间间隙。200ms对应10帧的音频
    }
    errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
    if (MSP_SUCCESS != errcode)
    {
        printf("\nQISRAudioWrite failed! error code:%d \n", errcode);
        goto iat_exit;
    }

    while (MSP_REC_STATUS_COMPLETE != rec_stat)
    {
        const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
        if (MSP_SUCCESS != errcode)
        {
            printf("\nQISRGetResult failed, error code: %d\n", errcode);
            goto iat_exit;
        }
        if (NULL != rslt)
        {
            unsigned int rslt_len = strlen(rslt);
            total_len += rslt_len;
            if (total_len >= BUFFER_SIZE)
            {
                printf("\nno enough buffer for rec_result !\n");
                goto iat_exit;
            }
            strncat(rec_result, rslt, rslt_len);
        }
        usleep(150 * 1000); // 防止频繁占用CPU
    }
    strcpy(result, rec_result);

iat_exit:
    if (NULL != f_pcm)
    {
        fclose(f_pcm);
        f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
        free(p_pcm);
        p_pcm = NULL;
    }

    QISRSessionEnd(session_id, hints);
}

void wav_to_str(char *buf)
{
    ret = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != ret)
    {
        printf("MSPLogin failed , Error code %d.\n", ret);
        goto exit; // 登录失败，退出登录
    }
    printf("\n########################################################################\n");
    printf("## 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。##\n");
    printf("########################################################################\n\n");

    run_iat("Client/test.wav", session_begin_params, buf); // 语音识别(导入test.wav文件，输出识别后字符串到result缓存区)
    printf("识别到内容为：%s\n", buf);
    system("rm ./Client/test.wav");

exit:
    MSPLogout(); // 退出登录

}

# Android-APM 
 基于webrtc apm源码封装接口提供AEC、NS、AGC、VAD功能，用于IM通信APP进行双向音频的优化   
```
typedef enum{
    XKAPM_DIR_NEAR_END = 0,//近端音频
    XKAPM_DIR_FAR_END,//远端音频
}XkAPM_Dir;

typedef struct{
    short* pData;//音频数据
    int iFrameLen;
    XkAPM_Dir iDir;
    long long iPts;//时间戳
}XkApm_Frame;

int XkApm_Ns(XkApm_Handle handle,XkApm_Frame* pFrame);//噪声消除
int XkApm_Aec(XkApm_Handle handle,XkApm_Frame* pFrame);//回声消除
int XkApm_Vad(XkApm_Handle handle,XkApm_Frame* pFrame);//语音检测
int XkApm_Agc(XkApm_Handle handle,XkApm_Frame* pFrame);//声音增益

```

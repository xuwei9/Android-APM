/*
 * XkApm.h
 *
 *  Created on: 2019-7-10
 *      Author: Admin
 */

#ifndef XKAPM_H_
#define XKAPM_H_

typedef enum{
    XKAPM_DIR_NEAR_END = 0,
    XKAPM_DIR_FAR_END,
}XkAPM_Dir;

typedef struct {
	bool bAecEnable;
	bool bAgcEnable;
	bool bNsEnable;
    int iAecDelay;
    int iAecMode;//enum { kAecNlpConservative = 0, kAecNlpModerate, kAecNlpAggressive };0-2
    int iVadMode;//Aggressiveness mode (0, 1, 2, or 3).
    int iNsMode;//0: Mild, 1: Medium , 2: Aggressive
    float fAgcDb;//0-49.f
}XkApm_Param;

typedef struct{
	short* pData;
	int iFrameLen;
	XkAPM_Dir iDir;
	long long iPts;
}XkApm_Frame;

typedef void* XkApm_Handle;

int XkApm_Open(XkApm_Handle* pHandle);
int XkApm_Close(XkApm_Handle handle);
int XkApm_Reset(XkApm_Handle handle);

int XkApm_Ns(XkApm_Handle handle,XkApm_Frame* pFrame);
int XkApm_Aec(XkApm_Handle handle,XkApm_Frame* pFrame);
int XkApm_Vad(XkApm_Handle handle,XkApm_Frame* pFrame);
int XkApm_Agc(XkApm_Handle handle,XkApm_Frame* pFrame);

int XkApm_Config(XkApm_Handle handle, XkApm_Param config);


#endif /* XKAPM_H_ */
